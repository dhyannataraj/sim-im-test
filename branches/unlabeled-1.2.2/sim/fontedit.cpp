/***************************************************************************
                          fontedit.cpp  -  description
                             -------------------
    begin                : Sun Mar 17 2002
    copyright            : (C) 2002 by Vladimir Shutoff
    email                : shutoff@mail.ru
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "fontedit.h"
#include "icons.h"

#include <qlayout.h>
#include <qpushbutton.h>
#include <qlabel.h>

#if USE_KDE
#include <kfontdialog.h>
#else
#include <qfontdialog.h>
#endif

FontEdit::FontEdit(QWidget *parent, const char *name) : QFrame(parent, name)
{
    QHBoxLayout *lay = new QHBoxLayout(this);
    lblFont = new QLabel("...", this);
    lblFont->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum));
    lay->addWidget(lblFont);
    lay->addSpacing(2);
    QPushButton *btnFont = new QPushButton(this);
    btnFont->setPixmap(Pict("text"));
    lay->addWidget(btnFont);
    lblFont->setFrameShape(QFrame::Box);
    lblFont->setLineWidth(1);
    lblFont->setMargin(3);
    connect(btnFont, SIGNAL(clicked()), this, SLOT(chooseFont()));
}

void FontEdit::setWinFont(const QFont &_f)
{
    f = _f;
    QString fontName = f.family();
    fontName += ", ";
    fontName += QString::number(f.pointSize());
    fontName += " pt.";
    switch (f.weight()){
    case QFont::Light:
        fontName += ", ";
        fontName += i18n("light");
        break;
    case QFont::DemiBold:
        fontName += ", ";
        fontName += i18n("demibold");
        break;
    case QFont::Bold:
        fontName += ", ";
        fontName += i18n("bold");
        break;
    case QFont::Black:
        fontName += ", ";
        fontName += i18n("black");
        break;
    default:
        break;
    }
    if (f.italic()){
        fontName += ", ";
        fontName += i18n("italic");
    }
    lblFont->setText(fontName);
}

void FontEdit::chooseFont()
{
#if USE_KDE
    QFont _f = f;
    if (KFontDialog::getFont(_f, false, topLevelWidget()) == KFontDialog::Accepted)
        setWinFont(_f);
#else
    bool ok = false;
    QFont _f = QFontDialog::getFont(&ok, f, topLevelWidget());
    if (ok) setWinFont(_f);
#endif
}

#ifndef _WINDOWS
#include "fontedit.moc"
#endif
