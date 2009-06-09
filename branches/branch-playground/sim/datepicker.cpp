/***************************************************************************
                          datepicker.cpp  -  description
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

#include "datepicker.h"
#include "icons.h"
#include "misc.h"
#include "toolbtn.h"

#include <qdatetime.h>
#include <qfontmetrics.h>
#include <qlineedit.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qpainter.h>
#include <qvalidator.h>
#include <QHBoxLayout>
#include <QPaintEvent>
#include <QGridLayout>
#include <QFrame>
#include <QLabel>
#include <QMouseEvent>
#include <QVBoxLayout>

class DateValidator : public QValidator
{
public:
    DateValidator(QWidget *parent);
    virtual State validate(QString &str, int &pos) const;
};

DateValidator::DateValidator(QWidget *parent)
        : QValidator(parent)
{
}

QValidator::State DateValidator::validate(QString &str, int&) const
{
    if (QDate::fromString(str, Qt::ISODate).isValid())
        return Acceptable;

    return Invalid;
}

class DateEdit : public QLineEdit
{
public:
    DateEdit(QWidget*);
};

DateEdit::DateEdit(QWidget *parent)
        : QLineEdit(parent)
{
    setValidator(new DateValidator(this));
    setInputMask("0000-00-00;_");
}

DatePicker::DatePicker(QWidget *parent)
        : QFrame(parent)
{
    setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum));
    setLineWidth(0);
    QHBoxLayout *lay = new QHBoxLayout(this);
    m_edit = new DateEdit(this);
    QFontMetrics fm(m_edit->font());
    m_edit->setFixedWidth(fm.width("0000-00-00") + 14);
    lay->addWidget(m_edit);
    m_button = new QPushButton(this);
    m_button->setIcon(SIM::Icon("more"));
    lay->addWidget(m_button);
    lay->addStretch();
    connect(m_button, SIGNAL(clicked()), this, SLOT(showPopup()));
    connect(m_edit, SIGNAL(textChanged(const QString&)), this, SLOT(textChanged(const QString&)));
}

DatePicker::~DatePicker()
{
}

void DatePicker::setEnabled(bool state)
{
    m_edit->setReadOnly(!state);
    m_button->setEnabled(state);
}

void DatePicker::setText(const QString &s)
{
	if (QDate::fromString(s, Qt::ISODate).isValid())
		m_edit->setText(s);
	else 
		m_edit->setText(QString::null);
}

QString DatePicker::text()
{
    return m_edit->text();
}

QDate DatePicker::getDate()
{
    return QDate::fromString(m_edit->text(), Qt::ISODate);
}

void DatePicker::paintEvent(QPaintEvent *e)
{
    if (parentWidget() && parentWidget()->backgroundPixmap())
	{
        QPoint pos = mapToParent(QPoint(0, 0));
        QPainter p(this);
        p.drawTiledPixmap(0, 0, width(), height(), *parentWidget()->backgroundPixmap(), pos.x(), pos.y());
        return;
    }
    QFrame::paintEvent(e);
}

void DatePicker::setDate(QDate date)
{
    m_edit->setText(date.toString(Qt::ISODate));
    emit changed();
}

void DatePicker::textChanged(const QString&)
{
    emit changed();
}

void DatePicker::showPopup()
{
    PickerPopup *popup = new PickerPopup(this);
    QSize s = popup->minimumSizeHint();
    popup->resize(s);
    QPoint p = CToolButton::popupPos(m_button, popup);
    popup->move(p);
    popup->show();
}

static const char *month_name[] =
    {
        I18N_NOOP("January"),
        I18N_NOOP("February"),
        I18N_NOOP("March"),
        I18N_NOOP("April"),
        I18N_NOOP("May"),
        I18N_NOOP("June"),
        I18N_NOOP("July"),
        I18N_NOOP("August"),
        I18N_NOOP("September"),
        I18N_NOOP("October"),
        I18N_NOOP("November"),
        I18N_NOOP("December")
    };

static const char *day_name[] =
    {
        I18N_NOOP("Mon"),
        I18N_NOOP("Tue"),
        I18N_NOOP("Wed"),
        I18N_NOOP("Thu"),
        I18N_NOOP("Fri"),
        I18N_NOOP("Sat"),
        I18N_NOOP("Sun"),
    };

class MonthSpinBox : public QSpinBox
{
public:
    MonthSpinBox(QWidget *p);
protected:
    QString textFromValue(int v) const;
};

MonthSpinBox::MonthSpinBox(QWidget *p)
        : QSpinBox(p)
{
}

QString MonthSpinBox::textFromValue(int v) const
{
    while (v < 0)
        v += 12;
    while (v >= 12)
        v -= 12;
    return i18n(month_name[v]);
}

PickerPopup::PickerPopup(DatePicker *picker)
        : QFrame(picker, Qt::Popup)
        , m_picker(picker)
{
    setAttribute(Qt::WA_DeleteOnClose);

    setFrameShape(QFrame::NoFrame);
    setFrameShadow(QFrame::Sunken);
    setLineWidth(1);

    QDate d = QDate::currentDate();
    QVBoxLayout *vLay = new QVBoxLayout(this);
    QHBoxLayout *hLay = new QHBoxLayout;
    vLay->addLayout(hLay);
    hLay->setMargin(0);
    hLay->setSpacing(4);

    m_monthBox = new MonthSpinBox(this);
    m_monthBox->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
    hLay->addWidget(m_monthBox);
    m_yearBox = new QSpinBox(this);
    m_yearBox->setMaximum(d.year());
    m_yearBox->setMinimum(d.year() - 200);
    m_yearBox->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed));
    hLay->addWidget(m_yearBox);
    connect(m_monthBox, SIGNAL(valueChanged(int)), this, SLOT(monthChanged(int)));
    connect(m_yearBox, SIGNAL(valueChanged(int)), this, SLOT(yearChanged(int)));

    QWidget *lbl = new QWidget(this);
    vLay->addWidget(lbl);
    vLay->setMargin(6);
    vLay->setSpacing(4);

    QPalette pal(palette());
    pal.setColor(QColorGroup::Text, QColor(127, 0, 0));
    pal.setColor(QColorGroup::Foreground, QColor(255, 0, 0));
    QFont f(font());
    f.setBold(true);

    QGridLayout *gLay = new QGridLayout;
    lbl->setLayout(gLay);
    unsigned n = 0;
	for (unsigned j = 0; j < 6; j++)
		for (unsigned i = 0; i < 7; i++)
		{
			QLabel *l = new PickerLabel(lbl);
			l->setFont(f);
			l->setAlignment(Qt::AlignRight);
			l->setText("99");
			l->setMinimumSize(l->sizeHint());
			l->setText(QString::number(n));
			gLay->addWidget(l, i, j + 1);
                        m_labels += l;
			if (i >= 5)
				l->setPalette(pal);
			connect(l, SIGNAL(clicked(PickerLabel*)), this, SLOT(dayClick(PickerLabel*)));
		}
    for (unsigned i = 0; i < 7; i++){
        QLabel *l = new QLabel(lbl);
        l->setFont(f);
        l->setText(i18n(day_name[i]));
        gLay->addWidget(l, i, 0);
        if (i >= 5)
            l->setPalette(pal);
    }
    int month = m_picker->getDate().month();
    int year = m_picker->getDate().year();

    if ((month == 0) || (year == 0))
	{
        month = d.month();
        year  = d.year();
    }
    m_monthBox->setValue(month - 1);
    m_yearBox->setValue(year);
    monthChanged(month - 1);
    yearChanged(year);
}

PickerPopup::~PickerPopup()
{
    qDeleteAll(m_labels);
}

void PickerPopup::dayClick(PickerLabel *lbl)
{
    unsigned year  = m_yearBox->text().toULong();
    unsigned month = m_monthBox->value() + 1;
    unsigned day   = lbl->text().toULong();
    QDate date;
    date.setYMD(year, month, day);
    m_picker->setDate(date);
    close();
}

void PickerPopup::monthChanged(int v)
{
    if (v < 0)
	{
        v += 12;
        m_yearBox->setValue(m_yearBox->text().toULong() - 1);
        m_monthBox->setValue(v);
    }
    if (v >= 12)
	{
        v -= 12;
        m_yearBox->setValue(m_yearBox->text().toULong() + 1);
        m_monthBox->setValue(v);
    }
    fill();
}

void PickerPopup::yearChanged(int v)
{
    QDate d = QDate::currentDate();
	if (v == d.year() - 200)
		m_monthBox->setMinimum(0);
	else
		m_monthBox->setMinimum(-1);
	if (v == d.year())
		m_monthBox->setMaximum(11);
	else
		m_monthBox->setMaximum(12);
    fill();
}

void PickerPopup::fill()
{
    int month = m_monthBox->value() + 1;
    if (month == 0)
        month += 12;
    if (month > 12)
        month -= 12;
    QDate d(m_yearBox->text().toULong(), month, 1);
    unsigned n = d.dayOfWeek() - 1;
    unsigned s = d.daysInMonth();
    unsigned i;
    for (i = 0; i < n; i++)
        m_labels[i]->setText(QString());
    for (i = 0; i < s; i++)
        m_labels[i + n]->setText(QString::number(i + 1));
    for (i = n + s; i < 42; i++)
        m_labels[i]->setText(QString());
}

PickerLabel::PickerLabel(QWidget *parent)
        : QLabel(parent)
{
}

void PickerLabel::mouseReleaseEvent(QMouseEvent*)
{
    emit clicked(this);
}
