#include "simapi.h"
/****************************************************************************
** Form implementation generated from reading ui file './livejournalcfgbase.ui'
**
** Created: вс дек 28 15:19:06 2003
**      by: The User Interface Compiler ($Id: livejournalcfgbase.cpp,v 1.1 2003-12-28 12:27:10 shutoff Exp $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#include "livejournalcfgbase.h"

#include <qvariant.h>
#include <qwidget.h>
#include <qpushbutton.h>
#include <qtabwidget.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qimage.h>
#include <qpixmap.h>

#include "linklabel.h"
static const char* const img0_livejournalcfgbase[] = { 
"22 22 2 1",
". c None",
"# c #a4c610",
"........######........",
".....###########......",
"....##############....",
"...################...",
"..######......######..",
"..#####........#####..",
".#####.......#..#####.",
".####.......###..####.",
"####.......#####..####",
"####......#####...####",
"####....#######...####",
"####....######....####",
"####...########...####",
".####.##########..####",
".####..####.#########.",
".#####..##...########.",
"..#####.......#######.",
"..######......######..",
"...###################",
"....##################",
"......###########.###.",
"........######.....#.."};

static const char* const img1_livejournalcfgbase[] = { 
"22 22 2 1",
". c None",
"# c #a1c410",
"........######........",
".....###########......",
"....##############....",
"...################...",
"..######......######..",
"..#####........#####..",
".#####.......#..#####.",
".####.......###..####.",
"####.......#####..####",
"####......#####...####",
"####....#######...####",
"####....######....####",
"####...########...####",
".####.##########..####",
".####..####.#########.",
".#####..##...########.",
"..#####.......#######.",
"..######......######..",
"...###################",
"....##################",
"......###########.###.",
"........######.....#.."};


/* 
 *  Constructs a LiveJournalCfgBase as a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f'.
 */
LiveJournalCfgBase::LiveJournalCfgBase( QWidget* parent, const char* name, WFlags fl )
    : QWidget( parent, name, fl ),
      image0( (const char **) img0_livejournalcfgbase ),
      image1( (const char **) img1_livejournalcfgbase )
{
    if ( !name )
	setName( "LiveJournalCfg" );
    LiveJournalCfgLayout = new QVBoxLayout( this, 11, 6, "LiveJournalCfgLayout"); 

    tabLiveJournal = new QTabWidget( this, "tabLiveJournal" );

    tabLogin = new QWidget( tabLiveJournal, "tabLogin" );
    tabLoginLayout = new QGridLayout( tabLogin, 1, 1, 11, 6, "tabLoginLayout"); 

    TextLabel1 = new QLabel( tabLogin, "TextLabel1" );
    TextLabel1->setProperty( "alignment", int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    tabLoginLayout->addWidget( TextLabel1, 0, 0 );

    edtName = new QLineEdit( tabLogin, "edtName" );

    tabLoginLayout->addWidget( edtName, 0, 1 );

    lblPassword = new QLabel( tabLogin, "lblPassword" );
    lblPassword->setProperty( "alignment", int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    tabLoginLayout->addWidget( lblPassword, 1, 0 );

    edtPassword = new QLineEdit( tabLogin, "edtPassword" );
    edtPassword->setProperty( "echoMode", "Password" );

    tabLoginLayout->addWidget( edtPassword, 1, 1 );
    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
    tabLoginLayout->addItem( spacer, 3, 0 );

    lblLnk = new LinkLabel( tabLogin, "lblLnk" );

    tabLoginLayout->addMultiCellWidget( lblLnk, 2, 2, 0, 1 );
    tabLiveJournal->insertTab( tabLogin, QString("") );

    tab = new QWidget( tabLiveJournal, "tab" );
    tabLayout = new QGridLayout( tab, 1, 1, 11, 6, "tabLayout"); 

    edtServer = new QLineEdit( tab, "edtServer" );

    tabLayout->addMultiCellWidget( edtServer, 0, 0, 1, 2 );

    TextLabel3 = new QLabel( tab, "TextLabel3" );
    TextLabel3->setProperty( "alignment", int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    tabLayout->addWidget( TextLabel3, 0, 0 );

    TextLabel4 = new QLabel( tab, "TextLabel4" );
    TextLabel4->setProperty( "alignment", int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    tabLayout->addWidget( TextLabel4, 1, 0 );

    edtPath = new QLineEdit( tab, "edtPath" );

    tabLayout->addMultiCellWidget( edtPath, 1, 1, 1, 2 );

    TextLabel5 = new QLabel( tab, "TextLabel5" );
    TextLabel5->setProperty( "alignment", int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    tabLayout->addWidget( TextLabel5, 2, 0 );

    edtPort = new QSpinBox( tab, "edtPort" );
    edtPort->setProperty( "maxValue", 65535 );
    edtPort->setProperty( "minValue", 1 );

    tabLayout->addWidget( edtPort, 2, 1 );
    QSpacerItem* spacer_2 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    tabLayout->addItem( spacer_2, 2, 2 );
    QSpacerItem* spacer_3 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
    tabLayout->addItem( spacer_3, 3, 0 );
    tabLiveJournal->insertTab( tab, QString("") );

    tab_2 = new QWidget( tabLiveJournal, "tab_2" );
    tabLayout_2 = new QGridLayout( tab_2, 1, 1, 11, 6, "tabLayout_2"); 

    TextLabel1_2 = new QLabel( tab_2, "TextLabel1_2" );
    TextLabel1_2->setProperty( "alignment", int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    tabLayout_2->addWidget( TextLabel1_2, 0, 0 );

    edtInterval = new QSpinBox( tab_2, "edtInterval" );

    tabLayout_2->addWidget( edtInterval, 0, 1 );

    TextLabel2_2 = new QLabel( tab_2, "TextLabel2_2" );
    TextLabel2_2->setProperty( "sizePolicy", QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)1, 0, 0, TextLabel2_2->sizePolicy().hasHeightForWidth() ) );

    tabLayout_2->addWidget( TextLabel2_2, 0, 2 );
    QSpacerItem* spacer_4 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
    tabLayout_2->addItem( spacer_4, 1, 1 );
    tabLiveJournal->insertTab( tab_2, QString("") );
    LiveJournalCfgLayout->addWidget( tabLiveJournal );
    languageChange();
    resize( QSize(330, 265).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );
}

/*
 *  Destroys the object and frees any allocated resources
 */
LiveJournalCfgBase::~LiveJournalCfgBase()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void LiveJournalCfgBase::languageChange()
{
    setProperty( "caption", i18n( "Form1" ) );
    TextLabel1->setProperty( "text", i18n( "Username:" ) );
    lblPassword->setProperty( "text", i18n( "Password:" ) );
    tabLiveJournal->changeTab( tabLogin, i18n( "&LiveJournal" ) );
    TextLabel3->setProperty( "text", i18n( "Server:" ) );
    TextLabel4->setProperty( "text", i18n( "Path:" ) );
    TextLabel5->setProperty( "text", i18n( "Port:" ) );
    tabLiveJournal->changeTab( tab, i18n( "&Network" ) );
    TextLabel1_2->setProperty( "text", i18n( "Check interval:" ) );
    TextLabel2_2->setProperty( "text", i18n( "minutes" ) );
    tabLiveJournal->changeTab( tab_2, i18n( "&Options" ) );
}

#include "livejournalcfgbase.moc"
