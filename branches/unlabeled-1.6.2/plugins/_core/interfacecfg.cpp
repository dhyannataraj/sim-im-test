/***************************************************************************
                          interfacecfg.cpp  -  description
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

#include "simapi.h"
#include "interfacecfg.h"
#include "userviewcfg.h"
#include "historycfg.h"
#include "msgcfg.h"
#include "smscfg.h"
#include "core.h"

#include <qtabwidget.h>
#include <qcombobox.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qspinbox.h>
#include <qlabel.h>

#ifndef USE_KDE

typedef struct language
{
    const char *code;
    const char *name;
} language;

static language langs[] =
    {
        { "-", I18N_NOOP("English") },
        { "bg", I18N_NOOP("Bulgarian") },
        { "ca", I18N_NOOP("Catalan") },
        { "cs", I18N_NOOP("Czech") },
        { "de", I18N_NOOP("German") },
        { "es", I18N_NOOP("Spanish") },
        { "fr", I18N_NOOP("French") },
        { "he", I18N_NOOP("Hebrew") },
        { "it", I18N_NOOP("Italian") },
        { "nl", I18N_NOOP("Dutch") },
        { "pl", I18N_NOOP("Polish") },
        { "ru", I18N_NOOP("Russian") },
        { "sk", I18N_NOOP("Slovak") },
        { "sw", I18N_NOOP("Swabian") },
        { "tr", I18N_NOOP("Turkish") },
        { "uk", I18N_NOOP("Ukrainian") },
        { "zh_TW", I18N_NOOP("Chinese") },
        { NULL, NULL }
    };
#endif

InterfaceConfig::InterfaceConfig(QWidget *parent)
        : InterfaceConfigBase(parent)
{
    for (QObject *p = parent; p != NULL; p = p->parent()){
        if (!p->inherits("QTabWidget"))
            continue;
        QTabWidget *tab = static_cast<QTabWidget*>(p);
        userview_cfg = new UserViewConfig(tab);
        tab->addTab(userview_cfg, i18n("Contact list"));
        history_cfg = new HistoryConfig(tab);
        tab->addTab(history_cfg, i18n("History"));
        void *data = getContacts()->getUserData(CorePlugin::m_plugin->user_data_id);
        msg_cfg = new MessageConfig(tab, data);
        tab->addTab(msg_cfg, i18n("Messages"));
        data = getContacts()->getUserData(CorePlugin::m_plugin->sms_data_id);
        sms_cfg = new SMSConfig(tab, data);
        tab->addTab(sms_cfg, i18n("SMS"));
        break;
    }
    const char *cur = CorePlugin::m_plugin->getLang();
    if (*cur == 0)
        cur = NULL;
#ifndef USE_KDE
    cmbLang->insertItem(i18n("System"));

    QStringList items;
    QString current;
    for (const language *l = langs; l->code; l++){
        items.append(i18n(l->name));
        if (cur && !strcmp(l->code, cur))
            current = i18n(l->name);
    }

    items.sort();
    cmbLang->insertStringList(items);
    if (!current.isEmpty()){
        unsigned n = 1;
        for (QStringList::Iterator it = items.begin(); it != items.end(); ++it, n++){
            if ((*it) == current){
                cmbLang->setCurrentItem(n);
                break;
            }
        }
    }else{
        cmbLang->setCurrentItem(0);
    }
#else
    cmbLang->hide();
#endif
    connect(grpMode, SIGNAL(clicked(int)), this, SLOT(modeChanged(int)));
    if (CorePlugin::m_plugin->getContainerMode()){
        grpMode->setButton(1);
        grpContainer->setButton(CorePlugin::m_plugin->getContainerMode() - 1);
        chkEnter->setChecked(CorePlugin::m_plugin->getSendOnEnter());
    }else{
        grpMode->setButton(0);
        grpContainer->setEnabled(false);
    }
    QString copy2;
    QString copy1 = i18n("Copy %1 messages from history");
    int n = copy1.find("%1");
    if (n >= 0){
        copy2 = copy1.mid(n + 2);
        copy1 = copy1.left(n);
    }
    lblCopy1->setText(copy1);
    lblCopy2->setText(copy2);
    spnCopy->setValue(CorePlugin::m_plugin->getCopyMessages());
}

InterfaceConfig::~InterfaceConfig()
{
}

void InterfaceConfig::modeChanged(int mode)
{
    if (mode){
        grpContainer->setEnabled(true);
        grpContainer->setButton(2);
    }else{
        QButton *btn = grpContainer->selected();
        if (btn)
            btn->toggle();
        chkEnter->setChecked(false);
        grpContainer->setEnabled(false);
    }
}

void InterfaceConfig::apply()
{
    userview_cfg->apply();
    history_cfg->apply();
    void *data = getContacts()->getUserData(CorePlugin::m_plugin->user_data_id);
    msg_cfg->apply(data);
    data = getContacts()->getUserData(CorePlugin::m_plugin->sms_data_id);
    sms_cfg->apply(data);
#ifndef USE_KDE
    int res = cmbLang->currentItem();
    const char *lang = "";
    if (res > 0){
        QStringList items;
        const language *l;
        for (l = langs; l->code; l++)
            items.append(i18n(l->name));
        items.sort();
        res--;
        QString current = items[res];
        for (l = langs; l->code; l++)
            if (i18n(l->name) == current){
                lang = l->code;
                break;
            }
    }
#endif
    if (grpMode->selected()){
        if (grpMode->id(grpMode->selected())){
            CorePlugin::m_plugin->setContainerMode(grpContainer->id(grpContainer->selected()) + 1);
            CorePlugin::m_plugin->setSendOnEnter(chkEnter->isChecked());
            CorePlugin::m_plugin->setCopyMessages(atol(spnCopy->text().latin1()));
        }else{
            CorePlugin::m_plugin->setContainerMode(0);
            CorePlugin::m_plugin->setSendOnEnter(false);
        }
    }
#ifndef USE_KDE
    if (!strcmp(lang, CorePlugin::m_plugin->getLang()))
        return;
    CorePlugin::m_plugin->removeTranslator();
    CorePlugin::m_plugin->setLang(lang);
    CorePlugin::m_plugin->installTranslator();
#endif
}

#ifndef WIN32
#include "interfacecfg.moc"
#endif

