/****************************************************************************
** Form interface generated from reading ui file './livejournalcfgbase.ui'
**
** Created: вс дек 28 15:12:44 2003
**      by: The User Interface Compiler ($Id: livejournalcfgbase.h,v 1.1 2003-12-28 12:27:10 shutoff Exp $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef LIVEJOURNALCFG_H
#define LIVEJOURNALCFG_H

#include <qvariant.h>
#include <qpixmap.h>
#include <qwidget.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class LinkLabel;
class QTabWidget;
class QLabel;
class QLineEdit;
class QSpinBox;

class LiveJournalCfgBase : public QWidget
{
    Q_OBJECT

public:
    LiveJournalCfgBase( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~LiveJournalCfgBase();

    QTabWidget* tabLiveJournal;
    QWidget* tabLogin;
    QLabel* TextLabel1;
    QLineEdit* edtName;
    QLabel* lblPassword;
    QLineEdit* edtPassword;
    LinkLabel* lblLnk;
    QWidget* tab;
    QLineEdit* edtServer;
    QLabel* TextLabel3;
    QLabel* TextLabel4;
    QLineEdit* edtPath;
    QLabel* TextLabel5;
    QSpinBox* edtPort;
    QWidget* tab_2;
    QLabel* TextLabel1_2;
    QSpinBox* edtInterval;
    QLabel* TextLabel2_2;

protected:
    QVBoxLayout* LiveJournalCfgLayout;
    QGridLayout* tabLoginLayout;
    QGridLayout* tabLayout;
    QGridLayout* tabLayout_2;

protected slots:
    virtual void languageChange();

private:
    QPixmap image0;
    QPixmap image1;

};

#endif // LIVEJOURNALCFG_H
