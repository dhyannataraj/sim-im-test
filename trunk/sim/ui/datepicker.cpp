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
#include "toolbtn.h"

#include <qlineedit.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qiconset.h>
#include <qspinbox.h>
#include <qfontmetrics.h>
#include <qpainter.h>
#include <qvalidator.h>
#include <qdatetime.h>

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

static bool parseDate(const QString &str, int &day, int &month, int &year)
{
    day   = 0;
    month = 0;
    year  = 0;
    int p;
    for (p = 0; p < (int)(str.length()); p++){
        QChar cc = str[p];
        char c = cc;
        if (c == '_')
            continue;
        if ((c < '0') || (c > '9')){
            p++;
            break;
        }
        day = day * 10 + (c - '0');
    }
    for (; p < (int)(str.length()); p++){
        QChar cc = str[p];
        char c = cc;
        if (c == '_')
            continue;
        if ((c < '0') || (c > '9')){
            p++;
            break;
        }
        month = month * 10 + (c - '0');
    }
    for (; p < (int)(str.length()); p++){
        QChar cc = str[p];
        char c = cc;
        if (c == '_')
            continue;
        if ((c < '0') || (c > '9'))
            return false;
        year = year * 10 + (c - '0');
    }
    if (day && month && year){
        QDate d(year, month, day);
        if (d.isNull())
            return false;
    }
    return true;
}

QValidator::State DateValidator::validate(QString &str, int&) const
{
    int day, month, year;
    if (!parseDate(str, day, month, year))
        return Invalid;
    if ((day == 0) && (month == 0) && (year == 0))
        return Acceptable;
    if ((day == 0) || (month == 0) || (year == 0))
        return Intermediate;
    return Acceptable;
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
#if QT_VERSION >= 0x030200
    setInputMask("00/00/0000;_");
#endif
}

DatePicker::DatePicker(QWidget *parent, const char *name)
        : QFrame(parent, name)
{
    setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum));
    setLineWidth(0);
    QHBoxLayout *lay = new QHBoxLayout(this);
    m_edit = new DateEdit(this);
    QFontMetrics fm(m_edit->font());
    m_edit->setFixedWidth(fm.width("00/00/0000") + 14);
    lay->addWidget(m_edit);
    m_button = new QPushButton(this);
    m_button->setPixmap(Pict("btn_more"));
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
    int day, month, year;
    if (!parseDate(s, day, month, year)){
        m_edit->setText("");
        return;
    }
    if ((day == 0) || (month == 0) || (year == 0)){
        m_edit->setText("");
        return;
    }
    m_edit->setText(s);
}

QString DatePicker::text()
{
    int day, month, year;
    getDate(day, month, year);
    if ((day == 0) || (month == 0) || (year == 0))
        return "";
    QString res;
    res.sprintf("%u/%02u/%04u", day, month, year);
    return res;
}

void DatePicker::paintEvent(QPaintEvent *e)
{
    if (parentWidget() && parentWidget()->backgroundPixmap()){
        QPoint pos = mapToParent(QPoint(0, 0));
        QPainter p(this);
        p.drawTiledPixmap(0, 0, width(), height(), *parentWidget()->backgroundPixmap(), pos.x(), pos.y());
        return;
    }
    QFrame::paintEvent(e);
}

void DatePicker::setDate(int day, int month, int year)
{
    QString text;
    QDate d;
    if (day && month && year)
        d.setYMD(year, month, day);
    if (!d.isNull())
        text.sprintf("%u/%02u/%04u", day, month, year);
    m_edit->setText(text);
    emit changed();
}

void DatePicker::textChanged(const QString&)
{
    emit changed();
}

void DatePicker::getDate(int &day, int &month, int &year)
{
    if (!parseDate(m_edit->text(), day, month, year)){
        day   = 0;
        month = 0;
        year  = 0;
    }
    if ((day == 0) || (month == 0) || (year == 0)){
        day   = 0;
        month = 0;
        year  = 0;
    }
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

static char *month_name[] =
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

static char *day_name[] =
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
    QString mapValueToText(int v);
};

MonthSpinBox::MonthSpinBox(QWidget *p)
        : QSpinBox(p)
{
}

QString MonthSpinBox::mapValueToText(int v)
{
    if (v < 0)
        v += 12;
    if (v >= 12)
        v -= 12;
    return i18n(month_name[v]);
}

PickerPopup::PickerPopup(DatePicker *picker)
        : QFrame(NULL, "calendar", WType_Popup | WStyle_Customize | WStyle_Tool | WDestructiveClose)
{
    m_picker = picker;

    setFrameShape(PopupPanel);
    setFrameShadow(Sunken);
    setLineWidth(1);

    QDate d = QDate::currentDate();
    QLabel *lbl = new QLabel(this);
    lbl->setBackgroundMode(PaletteBase);
    QVBoxLayout *l = new QVBoxLayout(this);
    QHBoxLayout *hLay = new QHBoxLayout(l);
    hLay->setMargin(0);
    hLay->setSpacing(4);

    m_monthBox = new MonthSpinBox(this);
    hLay->addWidget(m_monthBox);
    m_yearBox = new QSpinBox(this);
    m_yearBox->setMaxValue(d.year());
    m_yearBox->setMinValue(d.year() - 200);
    m_monthBox->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
    hLay->addWidget(m_yearBox);
    connect(m_monthBox, SIGNAL(valueChanged(int)), this, SLOT(monthChanged(int)));
    connect(m_yearBox, SIGNAL(valueChanged(int)), this, SLOT(yearChanged(int)));
    l->addWidget(lbl);
    l->setMargin(6);
    l->setSpacing(4);

    QPalette pal(palette());
    pal.setColor(QColorGroup::Text, QColor(127, 0, 0));
    pal.setColor(QColorGroup::Foreground, QColor(255, 0, 0));
    QFont f(font());
    f.setBold(true);

    m_labels = new QLabel*[7 * 6];
    QGridLayout *lay = new QGridLayout(lbl, 7, 7);
    lay->setMargin(6);
    lay->setSpacing(4);
    unsigned n = 0;
    for (unsigned j = 0; j < 6; j++){
        for (unsigned i = 0; i < 7; i++){
            QLabel *l = new PickerLabel(lbl);
            l->setFont(f);
            l->setAlignment(AlignRight);
            l->setText("99");
            l->setMinimumSize(l->sizeHint());
            l->setText(QString::number(n));
            l->setBackgroundMode(PaletteBase);
            lay->addWidget(l, i, j + 1);
            m_labels[n++] = l;
            if (i >= 5)
                l->setPalette(pal);
            connect(l, SIGNAL(clicked(PickerLabel*)), this, SLOT(dayClick(PickerLabel*)));
        }
    }
    for (unsigned i = 0; i < 7; i++){
        QLabel *l = new QLabel(lbl);
        l->setFont(f);
        l->setText(i18n(day_name[i]));
        l->setBackgroundMode(PaletteBase);
        lay->addWidget(l, i, 0);
        if (i >= 5)
            l->setPalette(pal);
    }
    int day, month, year;
    m_picker->getDate(day, month, year);
    if ((month == 0) || (year == 0)){
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
    delete[] m_labels;
}

void PickerPopup::dayClick(PickerLabel *lbl)
{
    unsigned year  = atol(m_yearBox->text().latin1());
    unsigned month = m_monthBox->value() + 1;
    unsigned day   = atol(lbl->text().latin1());
    m_picker->setDate(day, month, year);
    close();
}

void PickerPopup::monthChanged(int v)
{
    if (v < 0){
        v += 12;
        m_yearBox->setValue(atol(m_yearBox->text().latin1()) - 1);
        m_monthBox->setValue(v);
    }
    if (v >= 12){
        v -= 12;
        m_yearBox->setValue(atol(m_yearBox->text().latin1()) + 1);
        m_monthBox->setValue(v);
    }
    fill();
}

void PickerPopup::yearChanged(int v)
{
    QDate d = QDate::currentDate();
    if (v == d.year() - 200){
        m_monthBox->setMinValue(0);
    }else{
        m_monthBox->setMinValue(-1);
    }
    if (v == d.year()){
        m_monthBox->setMaxValue(11);
    }else{
        m_monthBox->setMaxValue(12);
    }
    fill();
}

void PickerPopup::fill()
{
    int month = m_monthBox->value() + 1;
    if (month == 0)
        month += 12;
    if (month > 12)
        month -= 12;
    QDate d(atol(m_yearBox->text().latin1()), month, 1);
    unsigned n = d.dayOfWeek() - 1;
    unsigned s = d.daysInMonth();
    unsigned i;
    for (i = 0; i < n; i++)
        m_labels[i]->setText("");
    for (i = 0; i < s; i++)
        m_labels[i + n]->setText(QString::number(i + 1));
    for (i = n + s; i < 42; i++)
        m_labels[i]->setText("");
}

PickerLabel::PickerLabel(QWidget *parent)
        : QLabel(parent)
{
}

void PickerLabel::mouseReleaseEvent(QMouseEvent*)
{
    emit clicked(this);
}

#ifndef WIN32
#include "datepicker.moc"
#endif

