#include "simapi.h"
/****************************************************************************
** Form implementation generated from reading ui file './msgjournalbase.ui'
**
** Created: вс дек 28 15:20:33 2003
**      by: The User Interface Compiler ($Id: msgjournalbase.cpp,v 1.1 2003-12-28 12:27:11 shutoff Exp $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#include "msgjournalbase.h"

#include <qvariant.h>
#include <qwidget.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qimage.h>
#include <qpixmap.h>

static const char* const img0_msgjournalbase[] = { 
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


/* 
 *  Constructs a MsgJournalBase as a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f'.
 */
MsgJournalBase::MsgJournalBase( QWidget* parent, const char* name, WFlags fl )
    : QWidget( parent, name, fl ),
      image0( (const char **) img0_msgjournalbase )
{
    if ( !name )
	setName( "MsgJournalBase" );
    setProperty( "sizePolicy", QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)1, 0, 0, sizePolicy().hasHeightForWidth() ) );
    MsgJournalBaseLayout = new QVBoxLayout( this, 11, 6, "MsgJournalBaseLayout"); 

    Layout1 = new QHBoxLayout( 0, 0, 6, "Layout1"); 

    TextLabel1 = new QLabel( this, "TextLabel1" );
    Layout1->addWidget( TextLabel1 );

    edtSubj = new QLineEdit( this, "edtSubj" );
    Layout1->addWidget( edtSubj );
    MsgJournalBaseLayout->addLayout( Layout1 );

    Layout3 = new QHBoxLayout( 0, 0, 6, "Layout3"); 

    TextLabel2 = new QLabel( this, "TextLabel2" );
    Layout3->addWidget( TextLabel2 );

    cmbSecurity = new QComboBox( FALSE, this, "cmbSecurity" );
    Layout3->addWidget( cmbSecurity );

    TextLabel3 = new QLabel( this, "TextLabel3" );
    Layout3->addWidget( TextLabel3 );

    cmbMood = new QComboBox( FALSE, this, "cmbMood" );
    cmbMood->setProperty( "sizePolicy", QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)0, 0, 0, cmbMood->sizePolicy().hasHeightForWidth() ) );
    Layout3->addWidget( cmbMood );
    MsgJournalBaseLayout->addLayout( Layout3 );
    languageChange();
    resize( QSize(295, 77).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );
}

/*
 *  Destroys the object and frees any allocated resources
 */
MsgJournalBase::~MsgJournalBase()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void MsgJournalBase::languageChange()
{
    setProperty( "caption", i18n( "Form1" ) );
    TextLabel1->setProperty( "text", i18n( "Subject:" ) );
    TextLabel2->setProperty( "text", i18n( "Security:" ) );
    cmbSecurity->clear();
    cmbSecurity->insertItem( i18n( "Public" ) );
    cmbSecurity->insertItem( i18n( "Private" ) );
    TextLabel3->setProperty( "text", i18n( "Mood:" ) );
    cmbMood->clear();
    cmbMood->insertItem( QString::null );
}

#include "msgjournalbase.moc"
