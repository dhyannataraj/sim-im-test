/***************************************************************************
                          intedit.cpp  -  description
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

#include "intedit.h"

#include <qgroupbox.h>
#include <qlayout.h>
#include <qlabel.h>

IntLineEdit::IntLineEdit(QWidget *parent) : QLineEdit(parent)
{
    id = 0;
}

void IntLineEdit::focusOutEvent(QFocusEvent *e)
{
    QLineEdit::focusOutEvent(e);
    emit focusOut();
}

void IntLineEdit::keyPressEvent(QKeyEvent *e)
{
    QLineEdit::keyPressEvent(e);
    if (e->key() == Qt::Key_Escape) emit escape();
}

#if COMPAT_QT_VERSION < 0x030000

QRegExpValidator::QRegExpValidator(const Qt3::QRegExp& rx, QWidget *parent)
    : QValidator(parent), r(rx)
{
}

QValidator::State QRegExpValidator::validate( QString& input, int& pos ) const
{
    if ( r.exactMatch(input) ) {
	return Acceptable;
    } else {
	if ( r.matchedLength() == (int) input.length() ) {
	    return Intermediate;
	} else {
	    pos = input.length();
	    return Invalid;
	}
    }
}

#define	QRegExp		Qt3::QRegExp

#endif

EMailValidator::EMailValidator(QWidget *parent)
: QRegExpValidator(QRegExp("[A-Za-z0-9\\.\\-_\\+]+@[A-Za-z0-9\\-_]+\\.[A-Za-z0-9\\.\\-_]+"), parent)
{
}

PhoneValidator::PhoneValidator(QWidget *parent)
: QRegExpValidator(QRegExp("\\+?[0-9 ]+(\\([0-9]+\\))?([0-9 ]+\\-)*[0-9 ]+"), parent)
{
}

const unsigned GRP_RADIO_POS	= 10;

GroupRadioButton::GroupRadioButton(const QString &text, QGroupBox *parent)
: QRadioButton(text, parent->parentWidget())
{
	QSize s = sizeHint();
	resize(s.width() + 3, s.height());
	move(parent->x() + GRP_RADIO_POS, parent->y() - s.height() / 2);
	parent->installEventFilter(this);
	if (parent->parentWidget()->layout() && parent->parentWidget()->layout()->inherits("QVBoxLayout")){
		QVBoxLayout *lay = static_cast<QVBoxLayout*>(parent->parentWidget()->layout());
		QLayoutIterator it = lay->iterator();
		int pos = lay->findWidget(parent);
		if (pos >= 0)
			lay->insertSpacing(pos, s.height() / 2);
	}
	setAutoMask(false);
}

bool GroupRadioButton::eventFilter(QObject *o, QEvent *e)
{
	bool res = QRadioButton::eventFilter(o, e);
	if (e->type() == QEvent::Move){
		QGroupBox *grp = static_cast<QGroupBox*>(o);
		QSize s = sizeHint();
		move(grp->x() + GRP_RADIO_POS, grp->y() - s.height() / 2);
	}
	return res;
}

#ifndef _WINDOWS
#include "intedit.moc"
#endif
