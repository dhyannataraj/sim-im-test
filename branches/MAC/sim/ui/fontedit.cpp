/***************************************************************************
                          fontedit.cpp  -  description
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

#include "fontedit.h"

#include <qlayout.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qregexp.h>

#ifdef USE_KDE
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
    lblFont->setText(font2str(f, true));
}

void FontEdit::setFont(const char *fontname)
{
    if (fontname){
        setWinFont(str2font(fontname, font()));
    }else{
        setWinFont(font());
    }
}

string FontEdit::getFont()
{
    string res;
    res = font2str(f, false).latin1();
    string base;
    base = font2str(font(), false).latin1();
    if (res == base)
        res = "";
    return res;
}

void FontEdit::chooseFont()
{
#ifdef USE_KDE
    QFont _f = f;
    if (KFontDialog::getFont(_f, false, topLevelWidget()) == KFontDialog::Accepted)
        setWinFont(_f);
#else
    bool ok = false;
    QFont _f = QFontDialog::getFont(&ok, f, topLevelWidget());
    if (ok) setWinFont(_f);
#endif
}

static QString s_tr(const char *s, bool use_tr)
{
    if (use_tr) return i18n(s);
    return s;
}

QString FontEdit::font2str(const QFont &f, bool use_tr)
{
    QString fontName = f.family();
    fontName += ", ";
    if (f.pointSize() > 0){
        fontName += QString::number(f.pointSize());
        fontName += " pt.";
    }else{
        fontName += QString::number(f.pixelSize());
        fontName += " pix.";
    }
    switch (f.weight()){
    case QFont::Light:
        fontName += ", ";
        fontName += s_tr(I18N_NOOP("light"), use_tr);
        break;
    case QFont::DemiBold:
        fontName += ", ";
        fontName += s_tr(I18N_NOOP("demibold"), use_tr);
        break;
    case QFont::Bold:
        fontName += ", ";
        fontName += s_tr(I18N_NOOP("bold"), use_tr);
        break;
    case QFont::Black:
        fontName += ", ";
        fontName += s_tr(I18N_NOOP("black"), use_tr);
        break;
    default:
        break;
    }
    if (f.italic()){
        fontName += ", ";
        fontName += s_tr(I18N_NOOP("italic"), use_tr);
    }
    if (f.strikeOut()){
        fontName += ", ";
        fontName += s_tr(I18N_NOOP("strikeout"), use_tr);
    }
    if (f.underline()){
        fontName += ", ";
        fontName += s_tr(I18N_NOOP("underline"), use_tr);
    }
    return fontName;
}

QFont FontEdit::str2font(const char *str, const QFont &def)
{
    QFont f(def);
    QStringList l = QStringList::split(QRegExp(" *, *"), QString::fromLocal8Bit(str));
    if (l.count() == 0) return f;
    int weight = QFont::Normal;
    bool italic    = false;
    bool strikeout = false;
    bool underline = false;
    f.setFamily(l[0]);
    for (unsigned i = 1; i < l.count(); i++){
        QString s = l[i];
        if (s == "italic"){
            italic = true;
            continue;
        }
        if (s == "strikeout"){
            strikeout = true;
            continue;
        }
        if (s == "underline"){
            underline = true;
            continue;
        }
        if (s == "light"){
            weight = QFont::Light;
            continue;
        }
        if (s == "demibold"){
            weight = QFont::DemiBold;
            continue;
        }
        if (s == "bold"){
            weight = QFont::Bold;
            continue;
        }
        if (s == "black"){
            weight = QFont::Black;
            continue;
        }
        int p = s.find(QRegExp("[0-9]+ *pt"));
        if (p >= 0){
            s = s.mid(p);
            int size = atol(s.latin1());
            if (size > 0)
                f.setPointSize(size);
            continue;
        }
        p = s.find(QRegExp("[0-9]+ *pix"));
        if (p >= 0){
            s = s.mid(p);
            int size = atol(s.latin1());
            if (size > 0)
                f.setPixelSize(size);
            continue;
        }
    }
    f.setItalic(italic);
    f.setUnderline(underline);
    f.setStrikeOut(strikeout);
    f.setWeight(weight);
    return f;
}

#ifndef _WINDOWS
#include "fontedit.moc"
#endif
