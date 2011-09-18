/***************************************************************************
                          icqinfo.cpp  -  description
                             -------------------
    begin                : Sun Mar 17 2002
    copyright            : (C) 2002 by Vladimir Shutoff
    email                : vovan@shutoff.ru
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "icqinfo.h"
#include "../icqclient.h"
#include "simgui/ballonmsg.h"
#include "contacts/contact.h"
#include "imagestorage/imagestorage.h"
#include "../metainfosnachandler.h"
#include "events/eventhub.h"

#include <QLineEdit>
#include <QStringList>
#include <QComboBox>
#include <QPixmap>
#include <QLabel>
#include <QTabWidget>

using namespace SIM;

const ext_info chat_groups[] =
    {
        { I18N_NOOP("General chat"), 1 },
        { I18N_NOOP("Romance"), 2 },
        { I18N_NOOP("Games"), 3 },
        { I18N_NOOP("Students"), 4 },
        { I18N_NOOP("20 Something"), 5 },
        { I18N_NOOP("30 Something"), 6 },
        { I18N_NOOP("40 Something"), 7 },
        { I18N_NOOP("50 Plus"), 8 },
        { I18N_NOOP("Seeking Women"), 9 },
        { I18N_NOOP("Seeking Men"), 10 },
        { "", 0 }
    };

const ext_info *p_chat_groups = chat_groups;

ICQInfo::ICQInfo(QWidget* parent, const ICQContactPtr& contact, ICQClient* client) : QWidget(parent)
{
    m_ui = new Ui::MainInfo;
	m_ui->setupUi(this);
    m_client	= client;
    m_contact	= contact;
    m_ui->edtUin->setReadOnly(true);

    if(contact != client->ownerContact())
    {
        m_ui->edtFirst->setReadOnly(true);
        m_ui->edtLast->setReadOnly(true);
        m_ui->edtNick->setReadOnly(true);
        m_ui->edtAutoReply->setReadOnly(true);
        m_ui->lblRandom->hide();
        m_ui->cmbRandom->hide();
        m_ui->tabWnd->removeTab(m_ui->tabWnd->indexOf(m_ui->password));
    }
    else
    {
        m_ui->edtAutoReply->hide();
        connect(this, SIGNAL(raise(QWidget*)), topLevelWidget(), SLOT(raisePage(QWidget*)));
    }
    m_ui->edtOnline->setReadOnly(true);
    m_ui->edtNA->setReadOnly(true);
    m_ui->edtExtIP->setReadOnly(true);
    m_ui->edtIntIP->setReadOnly(true);
    m_ui->edtClient->setReadOnly(true);
    fill();
    SIM::getEventHub()->getEvent("icq_contact_basic_info_updated")->connectTo(this, SLOT(contactBasicInfoUpdated(QString)));
}

void ICQInfo::contactBasicInfoUpdated(const QString& contactScreen)
{
    if(contactScreen != m_contact->getScreen())
        return;

    fill();
}

//
//void ICQInfo::apply()
//{
//    ICQUserData *data = m_data;
//    if (data == NULL){
//        if (m_client->getState() == Client::Connected){
//            QString errMsg;
//            QWidget *errWidget = edtCurrent;
//            if (!edtPswd1->text().isEmpty() || !edtPswd2->text().isEmpty()){
//                if (edtCurrent->text().isEmpty()){
//                    errMsg = i18n("Input current password");
//                }else{
//                    if (edtPswd1->text() != edtPswd2->text()){
//                        errMsg = i18n("Confirm password does not match");
//                        errWidget = edtPswd2;
//                    }else if (edtCurrent->text() != m_client->getPassword()){
//                        errMsg = i18n("Invalid password");
//                    }
//                }
//            }
//            if (!errMsg.isEmpty()){
//                for (QWidget *p = parentWidget(); p; p = p->parentWidget()){
//                    if (p->inherits("QTabWidget")){
//                        static_cast<QTabWidget*>(p)->setCurrentWidget(this);
//                        break;
//                    }
//                }
//                emit raise(this);
//                BalloonMsg::message(errMsg, errWidget);
//                return;
//            }
//            if (!edtPswd1->text().isEmpty())
//                m_client->changePassword(edtPswd1->text());
//            // clear Textboxes
//            edtCurrent->clear();
//            edtPswd1->clear();
//            edtPswd2->clear();
//        }
//        m_data = &m_client->data.owner;
//        m_client->setRandomChatGroup(getComboValue(cmbRandom, chat_groups));
//    }
//}
//
//void ICQInfo::updateData(ICQUserData* data)
//{
//    data->setFirstName(edtFirst->text());
//    data->setLastName(edtLast->text());
//    data->setNick(edtNick->text());
//}
//
//void ICQInfo::applyContact(const SIM::ClientPtr& client, SIM::IMContact* contact)
//{
//    if (client != m_client)
//        return;
//    updateData(m_client->toICQUserData(contact));
//}
//
//void ICQInfo::apply(Client *client, void *_data)
//{
//    if (client != m_client)
//        return;
//    ICQUserData *data = m_client->toICQUserData((SIM::IMContact*)_data);  // FIXME unsafe type conversion
//    updateData(data);
//}
//
//bool ICQInfo::processEvent(Event *e)
//{
//    if (e->type() == eEventContact){
//        EventContact *ec = static_cast<EventContact*>(e);
//        if(ec->action() != EventContact::eChanged)
//            return false;
//        Contact *contact = ec->contact();
//        if (contact->have(m_data))
//            fill();
//    } else
//    if ((e->type() == eEventMessageReceived) && m_data){
//        EventMessage *em = static_cast<EventMessage*>(e);
//        Message *msg = em->msg();
//        if (msg->type() == MessageStatus){
//            if (m_client->dataName(m_data) == msg->client())
//                fill();
//        }
//    } else
//    if ((e->type() == eEventClientChanged) && (m_data == 0)){
//        EventClientChanged *ecc = static_cast<EventClientChanged*>(e);
//        if (ecc->client() == m_client)
//            fill();
//    }
//    return false;
//}

Ui::MainInfo* ICQInfo::ui() const
{
    return m_ui;
}

void ICQInfo::fill()
{
    m_ui->edtUin->setText(m_contact->getScreen());
    m_ui->edtFirst->setText(m_contact->getFirstName());
    m_ui->edtLast->setText(m_contact->getLastName());
    m_ui->edtNick->setText(m_contact->getNick());

    if(m_contact == m_client->ownerContact())
    {
        if (m_ui->edtFirst->text().isEmpty()) {
            QString firstName = "No name"; // FIXME get global
            m_ui->edtFirst->setText(firstName);
        }
        if (m_ui->edtLast->text().isEmpty()) {
            QString lastName = "No last name";
            m_ui->edtLast->setText(lastName);
        }
        m_ui->password->setEnabled(!m_client->currentStatus()->flag(SIM::IMStatus::flOffline));
    }

    m_ui->cmbStatus->clear();
    ICQStatusPtr status;
    if(m_contact != m_client->ownerContact())
    {
        status = m_contact->icqStatus();
    }
    else
    {
        status = m_client->currentIcqStatus();
        //initCombo(cmbRandom, m_client->getRandomChatGroup(), chat_groups);
    }

    if(m_contact != m_client->ownerContact())
    {
        m_ui->edtAutoReply->setPlainText(status->text());
    }
    else
    {
        m_ui->edtAutoReply->hide();
    }

    int current = 0;
    QString text;
    if((m_contact != m_client->ownerContact()) && (status->flag(SIM::IMStatus::flInvisible)))
    {
        m_ui->cmbStatus->addItem(SIM::getImageStorage()->icon("ICQ_invisible"), i18n("Possibly invisible"));
    }
    else
    {
        // Add moar
        m_ui->cmbStatus->addItem(QIcon(status->icon()), status->name());
    }

    disableWidget(m_ui->cmbStatus);
    if(status->flag(SIM::IMStatus::flOffline))
    {
        m_ui->lblOnline->setText(i18n("Last online") + ':');
        m_ui->edtOnline->setText(QDateTime::fromTime_t(m_contact->getStatusTime()).toString());
        m_ui->lblNA->hide();
        m_ui->edtNA->hide();
    }
    else
    {
		if(m_contact->getOnlineTime())
		{
		    m_ui->edtOnline->setText(QDateTime::fromTime_t(m_contact->getOnlineTime()).toString());
        }
		else
		{
		    m_ui->lblOnline->hide();
		    m_ui->edtOnline->hide();
        }
    }
    /*
    if (data->getIP()){
        edtExtIP->setText(formatAddr(data->getIP(), data->getPort()));
    }else{
        lblExtIP->hide();
        edtExtIP->hide();
    }
    if ((data->getRealIP()) && ((data->getIP() == 0) || ((data->getIP()) != (data->getRealIP())))){
        edtIntIP->setText(formatAddr(data->getRealIP(), data->getPort()));
    }else{
        lblIntIP->hide();
        edtIntIP->hide();
    }
    if (m_data){
        QString client_name = m_client->clientName(data);
        if (client_name.length()){
            edtClient->setText(client_name);
        }else{
            lblClient->hide();
            edtClient->hide();
        }
    }else{
        QString name = PACKAGE;
        name += ' ';
        name += VERSION;
#ifdef WIN32
        name += "/win32";
#endif
        edtClient->setText(name);
    }
    */
}

