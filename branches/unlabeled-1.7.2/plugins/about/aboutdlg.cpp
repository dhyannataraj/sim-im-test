/***************************************************************************
                          aboutdlg.cpp  -  description
                             -------------------
    begin                : Sun Mar 24 2002
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

#include "aboutdlg.h"
#include "about.h"
#include "simapi.h"
#include "textshow.h"
#include "aboutdata.h"
#ifndef USE_KDE

#include <qlabel.h>
#include <qpushbutton.h>
#include <qregexp.h>
#include <qtabwidget.h>
#include <qfile.h>
#include <qlineedit.h>

KAboutApplication::KAboutApplication( const KAboutData *aboutData, QWidget *parent, const char *name, bool modal)
        : AboutDlgBase(parent, name, modal)
{
    SET_WNDPROC("about")
    setButtonsPict(this);
    setCaption(caption());

    connect(btnOK, SIGNAL(clicked()), this, SLOT(close()));
    setIcon(Pict("ICQ"));
    const QIconSet *icon = Icon("ICQ");
    if (icon)
        lblIcon->setPixmap(icon->pixmap(QIconSet::Large, QIconSet::Normal));
    edtVersion->setText(i18n("%1 Version: %2") .arg(aboutData->appName()) .arg(aboutData->version()));
	edtVersion->setReadOnly(true);
	QPalette p = palette();
	p.setColor(QColorGroup::Base, colorGroup().background());
	edtVersion->setPalette(p);
    txtAbout->setText((QString("<center><br>%1<br><br>%2<br><br>") +
                       "<a href=\"%3\">%4</a><br><br>" +
                       i18n("Bug report") + ": <a href=\"mailto:%5\">%6</a><br>" +
                       i18n("Note: This is english mailing list") +
                       "</center>")
                      .arg(quote(aboutData->shortDescription()))
                      .arg(quote(aboutData->copyrightStatement()))
                      .arg(quote(aboutData->homepage()))
                      .arg(quote(aboutData->homepage()))
                      .arg(quote(aboutData->bugAddress()))
                      .arg(quote(aboutData->bugAddress())));
    QString txt;
    QValueList<KAboutPerson>::ConstIterator it;
    for (it = aboutData->authors().begin();
            it != aboutData->authors().end(); ++it)
    {
        txt += addPerson(&(*it));
        txt += "<br>";
    }
    txtAuthors->setText(txt);
    txt = "";
    QValueList<KAboutTranslator> translators = aboutData->translators();
    QValueList<KAboutTranslator>::ConstIterator itt;
    if (!translators.isEmpty()){
        for (itt = translators.begin();
                itt != translators.end(); ++itt)
        {
            const KAboutTranslator &t = *itt;
            txt += QString("%1 &lt;<a href=\"mailto:%2\">%3</a>&gt;<br>")
                   .arg(quote(t.name()))
                   .arg(quote(t.emailAddress()))
                   .arg(quote(t.emailAddress()));
            txt += "<br>";
        }
        txtTranslations->setText(txt);
    }else{
        tabMain->removePage(tabTranslation);
    }
    QString license = aboutData->license();
    license += "\n\n";
    QFile f(QFile::decodeName(app_file("COPYING").c_str()));
    if (f.open(IO_ReadOnly)){
        for (;;){
            QString s;
            if (f.readLine(s, 512) == -1)
                break;
            license += s;
        }
    }
    txtLicence->setText(quote(license));
}

KAboutApplication::~KAboutApplication()
{
}

void KAboutApplication::closeEvent(QCloseEvent *e)
{
    AboutDlgBase::closeEvent(e);
    emit finished();
}

QString KAboutApplication::addPerson(const KAboutPerson *p)
{
    QString res;
    if (!p->task().isEmpty()){
        res += quote(p->task());
        res += ":<br>";
    }
    res += QString("%1 &lt;<a href=\"mailto:%2\">%3</a>&gt;<br>")
           .arg(quote(p->name()))
           .arg(quote(p->emailAddress()))
           .arg(quote(p->emailAddress()));
    if (!p->webAddress().isEmpty())
        res += QString("<a href=\"%1\">%2</a><br>")
               .arg(quote(p->webAddress()))
               .arg(quote(p->webAddress()));
    return res;
}

QString KAboutApplication::quote(const QString &s)
{
    QString res = s;
    res.replace(QRegExp("&"), "&amp;");
    res.replace(QRegExp("\""), "&quot;");
    res.replace(QRegExp("<"), "&lt;");
    res.replace(QRegExp(">"), "&gt;");
    res.replace(QRegExp("\\n"), "<br>");
    return res;
}

#ifndef _WINDOWS
#include "aboutdlg.moc"
#endif
#endif

