/****************************************************************************
** Form interface generated from reading ui file './msgjournalbase.ui'
**
** Created: вс дек 28 15:12:45 2003
**      by: The User Interface Compiler ($Id: msgjournalbase.h,v 1.1 2003-12-28 12:27:11 shutoff Exp $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef MSGJOURNALBASE_H
#define MSGJOURNALBASE_H

#include <qvariant.h>
#include <qpixmap.h>
#include <qwidget.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QLabel;
class QLineEdit;
class QComboBox;

class MsgJournalBase : public QWidget
{
    Q_OBJECT

public:
    MsgJournalBase( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~MsgJournalBase();

    QLabel* TextLabel1;
    QLineEdit* edtSubj;
    QLabel* TextLabel2;
    QComboBox* cmbSecurity;
    QLabel* TextLabel3;
    QComboBox* cmbMood;

protected:
    QVBoxLayout* MsgJournalBaseLayout;
    QHBoxLayout* Layout1;
    QHBoxLayout* Layout3;

protected slots:
    virtual void languageChange();

private:
    QPixmap image0;

};

#endif // MSGJOURNALBASE_H
