/***************************************************************************
                          xpstyle.cpp  -  description
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

#include "xpstyle.h"

#include <windows.h>
#include <uxtheme.h>
#include <tmschema.h>

#include <qmap.h>
#include <qwidget.h>
#include <qpixmap.h>
#include <qapplication.h>
#include <qpainter.h>
#include <qtabbar.h>
#include <qheader.h>
#include <qslider.h>

#define Q_ASSERT ASSERT

static ulong ref = 0;
static bool use_xp  = FALSE;
static bool init_xp = FALSE;
static QMap<QString,HTHEME> *handleMap = 0;

class QWindowsXPStylePrivate
{
public:
    QWindowsXPStylePrivate()
	: hotWidget( 0 ), hotTab( 0 ), hotSpot( -1, -1 )
    {
	init();
    }
    ~QWindowsXPStylePrivate()
    {
	cleanup();
    }

    void init()
    {
        if ( !init_xp ) {
	    init_xp = TRUE;
	    use_xp = IsThemeActive() && IsAppThemed();
	}
	if ( use_xp )
	    ref++;
    }

    void cleanup()
    {
	init_xp = FALSE;
	if ( use_xp ) {
	    if ( !--ref ) {		
		use_xp  = FALSE;
		delete limboWidget;
		limboWidget = 0;
		delete tabbody;
		tabbody = 0;
		if ( handleMap ) {
		    QMap<QString, HTHEME>::Iterator it;
		    for ( it = handleMap->begin(); it != handleMap->end(); ++it )
			CloseThemeData( it.data() );
		    delete handleMap;
		    handleMap = 0;
		}		
	    }
	}
    }
    
    static bool getThemeResult( HRESULT res )
    {
	if ( res == S_OK )
	    return TRUE;
	return FALSE;
    }
    
    static HWND winId( const QWidget *widget )
    {
	if ( widget )
	    return widget->winId();

	if ( currentWidget )
	    return currentWidget->winId();

	if ( !limboWidget )
	    limboWidget = new QWidget( 0, "xp_limbo_widget" );

	return limboWidget->winId();
    }
    
    const QPixmap *tabBody( QWidget *widget );

    // hot-widget stuff

    const QWidget *hotWidget;
    static const QWidget *currentWidget;

    QTab *hotTab;
    QRect hotHeader;

    QPoint hotSpot;

private:
    static QWidget *limboWidget;
    static QPixmap *tabbody;
};

const QWidget *QWindowsXPStylePrivate::currentWidget = 0;
QWidget *QWindowsXPStylePrivate::limboWidget = 0;
QPixmap *QWindowsXPStylePrivate::tabbody = 0;

struct XPThemeData
{
    XPThemeData( const QWidget *w = 0, QPainter *p = 0, const QString &theme = QString::null, int part = 0, int state = 0, const QRect &r = QRect() )
        : widget( w ), painter( p ), name( theme ),partId( part ), stateId( state ), rec( r ), htheme( 0 )
    {
    }
    ~XPThemeData()
    {
    }
    
    HTHEME handle()
    {
	if ( !use_xp )
	    return NULL;
	
	if ( !htheme && handleMap )
	    htheme = handleMap->operator[]( name );

        if ( !htheme ) {
	    static wchar_t nm[256];
	    Q_ASSERT( name.length() < 255 );
	    memcpy( nm, (wchar_t*)name.unicode(), sizeof(wchar_t)*name.length() );
	    nm[name.length()] = 0;
            htheme = OpenThemeData( QWindowsXPStylePrivate::winId( widget ), nm );
	    if ( htheme ) {
		if ( !handleMap )
		    handleMap = new QMap<QString, HTHEME>;
		handleMap->operator[]( name ) = htheme;
	    }
	}
	
        return htheme;
    }
    
    bool isValid()
    {
	return use_xp && !!name && handle();
    }
    
    RECT rect()
    {
        RECT r;
        r.left = rec.x();
        r.right = rec.x() + rec.width();
        r.top = rec.y();
        r.bottom = rec.y() + rec.height();
	
        return r;
    }
    
    HRGN mask()
    {
	if ( IsThemeBackgroundPartiallyTransparent( handle(), partId, stateId ) ) {
	    HRGN hrgn;
//	    GetThemeBackgroundRegion( handle(), painter ? painter->handle() : 0, partId, stateId, &rect(), &hrgn );
		GetThemeBackgroundRegion( handle(), partId, stateId, &rect(), &hrgn );
	    return hrgn;
	}
	return 0;
    }

    void setTransparency()
    {
	HRGN hrgn = mask();
	if ( hrgn )
	    SetWindowRgn( QWindowsXPStylePrivate::winId( widget ), hrgn, FALSE );
    }

    void drawBackground( int pId = 0, int sId = 0 )
    {
	if ( pId )
	    partId = pId;
	if ( sId )
	    stateId = sId;

	if ( name && name == "TAB" && (
	    partId == TABP_TABITEMLEFTEDGE ||
	    partId == TABP_TABITEMRIGHTEDGE ||
	    partId == TABP_TABITEM ) ) {
	    QRect oldrec = rec;
	    rec = QRect( 0, 0, rec.width(), rec.height() );
	    QPixmap pm( rec.size() );
	    QPainter p( &pm );
	    p.eraseRect( 0, 0, rec.width(), rec.height() );
	    DrawThemeBackground( handle(), p.handle(), partId, stateId, &rect(), 0 );
	    rec = oldrec;
	    painter->drawPixmap( rec.x(), rec.y(), pm );
	} else {
	    ulong res = DrawThemeBackground( handle(), painter->handle(), partId, stateId, &rect(), 0 );
	}
    }

    int partId;
    int stateId;
    QRect rec;

private:
    const QWidget *widget;
    QPainter *painter;
    QString name;
    HTHEME htheme;
    bool workAround;
};

QWindowsXPStyle::QWindowsXPStyle()
: QWindowsStyle()
{
    d = new QWindowsXPStylePrivate;
}

QWindowsXPStyle::~QWindowsXPStyle()
{
    delete d;
}

void QWindowsXPStyle::unPolish( QApplication *app )
{
    d->cleanup();
    QWindowsStyle::unPolish( app );
}

void QWindowsXPStyle::polish( QApplication *app )
{
    static bool isPolished = FALSE;
    if ( !isPolished )
	ref--;
    QWindowsStyle::polish( app );
    init_xp = FALSE;
    d->init();
    isPolished = TRUE;
}

void QWindowsXPStyle::polish( QWidget *widget )
{
    QWindowsStyle::polish( widget );
    if ( !use_xp )
	return;

    if ( widget->inherits( "QButton" ) ) {
	widget->installEventFilter( this );
	widget->setBackgroundOrigin( QWidget::ParentOrigin );
    } else if ( widget->inherits( "QTabBar" ) ) {
	widget->installEventFilter( this );
	widget->setMouseTracking( TRUE );
	connect( widget, SIGNAL(selected(int)), this, SLOT(activeTabChanged()) );
    } else if ( widget->inherits( "QHeader" ) ) {
	widget->installEventFilter( this );
	widget->setMouseTracking( TRUE );
    } else if ( widget->inherits( "QComboBox" ) ) {
	widget->installEventFilter( this );
	widget->setMouseTracking( TRUE );
    } else if ( widget->inherits( "QSpinWidget" ) ) {
	widget->installEventFilter( this );
	widget->setMouseTracking( TRUE );
    } else if ( widget->inherits( "QScrollBar" ) ) {
	widget->installEventFilter( this );
	widget->setMouseTracking( TRUE );
    } else if ( widget->inherits( "QTitleBar" ) ) {
	widget->installEventFilter( this );
	widget->setMouseTracking( TRUE );
    } else if ( widget->inherits( "QWorkspaceChild" ) ) {
	widget->installEventFilter( this );
    } else if ( widget->inherits( "QSlider" ) ) {
	widget->installEventFilter( this );
	widget->setMouseTracking( TRUE );
    } else if ( widget->inherits( "QWidgetStack" ) &&
		widget->parentWidget() &&
		widget->parentWidget()->inherits( "QTabWidget" ) ) {
	widget->setPaletteBackgroundPixmap( *d->tabBody( widget ) );
    } else if ( widget->inherits( "QMenuBar" ) ) {
	QPalette pal = widget->palette();

	XPThemeData theme( widget, 0, "MENUBAR", 0, 0 );
	if ( theme.isValid() ) {
	    COLORREF cref;
	    GetThemeColor( theme.handle(), 0, 0, TMT_MENUBAR, &cref );
	    QColor menubar( qRgb(GetRValue(cref),GetGValue(cref),GetBValue(cref)) );
	    pal.setColor( QColorGroup::Button, menubar );
	} else {
	    QPalette apal = QApplication::palette();
	    pal.setColor( QPalette::Active, QColorGroup::Button, apal.color( QPalette::Active, QColorGroup::Button ) );
	    pal.setColor( QPalette::Inactive, QColorGroup::Button, apal.color( QPalette::Inactive, QColorGroup::Button ) );
	    pal.setColor( QPalette::Disabled, QColorGroup::Button, apal.color( QPalette::Disabled, QColorGroup::Button ) );
	}
	widget->setPalette( pal );
    }

    updateRegion( widget );
}

void QWindowsXPStyle::unPolish( QWidget *widget )
{
    widget->removeEventFilter( this );
    if ( widget->inherits( "QTitleBar" ) && !widget->inherits( "QDockWindowTitleBar" ) ) {
	SetWindowRgn( widget->winId(), 0, TRUE );
	if ( widget->isMinimized() ) {
	    SetWindowRgn( widget->parentWidget()->winId(), 0, TRUE );
	}
    } else if ( widget->inherits( "QWorkspaceChild" ) ) {
	SetWindowRgn( widget->winId(), 0, TRUE );
    } else if ( widget->inherits( "QWidgetStack" ) &&
		widget->parentWidget() &&
		widget->parentWidget()->inherits( "QTabWidget" ) ) {
	widget->setPaletteBackgroundPixmap( QPixmap() );
    } else if ( widget->inherits( "QTabBar" ) ) {
	disconnect( widget, SIGNAL(selected(int)), this, SLOT(activeTabChanged()) );
    }
    QWindowsStyle::unPolish( widget );
}

void QWindowsXPStyle::updateRegion( QWidget *widget )
{
    if ( !use_xp )
	return;

    if ( widget->inherits( "QTitleBar" ) && !widget->inherits( "QDockWindowTitleBar" ) ) {
	if ( widget->isMinimized() ) {
	    XPThemeData theme( widget, 0, "WINDOW", WP_MINCAPTION, CS_ACTIVE, widget->rect() );
	    theme.setTransparency();
	    XPThemeData theme2( widget->parentWidget(), 0, "WINDOW", WP_MINCAPTION, CS_ACTIVE, widget->rect() );
	    theme2.setTransparency();
	} else {
	    int partId = WP_CAPTION;
	    if ( widget->inherits( "QDockWindowTitleBar" ) )
		partId = WP_SMALLCAPTION;
	    XPThemeData theme( widget, 0, "WINDOW", partId, CS_ACTIVE, widget->rect() );
	    theme.setTransparency();
	}
    } else if ( widget->inherits( "QWorkspaceChild" ) ) {
	XPThemeData theme( widget, 0, "WINDOW", WP_CAPTION, CS_ACTIVE, widget->rect() );
	theme.setTransparency();
	theme.rec = widget->parentWidget()->rect();
	RECT r = theme.rect();
	InvalidateRect( widget->parentWidget()->winId(), &r, TRUE );
    }
}

// HotSpot magic
/*! \reimp */
bool QWindowsXPStyle::eventFilter( QObject *o, QEvent *e )
{
    if ( !o || !o->isWidgetType() || e->type() == QEvent::Paint || !use_xp)
	return QWindowsStyle::eventFilter( o, e );

    QWidget *widget = (QWidget*)o;

    switch ( e->type() ) {
    case QEvent::MouseMove:
	{
	    if ( !widget->isActiveWindow() )
		break;

	    QMouseEvent *me = (QMouseEvent*)e;

	    d->hotWidget = widget;
	    d->hotSpot = me->pos();

	    if ( o->inherits( "QTabBar" ) ) {
		QTabBar* bar = (QTabBar*)o;
		QTab * t = bar->selectTab( me->pos() );
		if ( d->hotTab != t ) {
		    d->hotTab = t;
		    widget->repaint( FALSE );
		}
	    } else if ( o->inherits( "QHeader" ) ) {
		QHeader *header = (QHeader*)o;
		QRect oldHeader = d->hotHeader;
		
		if ( header->orientation() == Horizontal )
		    d->hotHeader = header->sectionRect( header->sectionAt( d->hotSpot.x() ) );
		else
		    d->hotHeader = header->sectionRect( header->sectionAt( d->hotSpot.y() ) );
		
		if ( oldHeader != d->hotHeader ) {
		    if ( oldHeader.isValid() )
			header->update( oldHeader );
		    if ( d->hotHeader.isValid() )
			header->update( d->hotHeader );
		}
	    } else if ( o->inherits( "QTitleBar" ) ) {
		static SubControl clearHot = SC_TitleBarLabel;
		QTitleBar *titlebar = (QTitleBar*)o;
		SubControl sc = querySubControl( CC_TitleBar, titlebar, d->hotSpot );
		if ( sc != clearHot || clearHot != SC_TitleBarLabel ) {
		    QRect rect = visualRect( querySubControlMetrics( CC_TitleBar, titlebar, clearHot ), titlebar );
		    titlebar->repaint( rect, FALSE );

		    clearHot = sc;
		    rect = visualRect( querySubControlMetrics( CC_TitleBar, titlebar, sc ), titlebar );
		    titlebar->repaint( rect, FALSE );
		}
	    } else if ( o->inherits( "QSlider" ) ) {
		static clearSlider = FALSE;
		QSlider *slider = (QSlider*)o;
		const QRect rect = slider->sliderRect();
		const bool inSlider = rect.contains( d->hotSpot );
		if ( ( inSlider && !clearSlider ) || ( !inSlider && clearSlider ) ) {
		    clearSlider = inSlider;
		    slider->repaint( rect, FALSE );
		}
	    } else if ( o->inherits( "QComboBox" ) ) {
		static clearCombo = FALSE;
		const QRect rect = querySubControlMetrics( CC_ComboBox, (QWidget*)o, SC_ComboBoxArrow );
		const bool inArrow = rect.contains( d->hotSpot );
		if ( ( inArrow && !clearCombo ) || ( !inArrow && clearCombo ) ) {
		    clearCombo = inArrow;
		    widget->repaint( rect, FALSE );
		}
	    } else {
		widget->repaint( FALSE );
	    }
	}
        break;

    case QEvent::Enter:
	if ( !widget->isActiveWindow() )
	    break;
        d->hotWidget = widget;
        widget->repaint( FALSE );
        break;

    case QEvent::Leave:
	if ( !widget->isActiveWindow() )
	    break;
        if ( widget == d->hotWidget) {
            d->hotWidget = 0;
	    d->hotHeader = QRect();
	    d->hotTab = 0;
            widget->repaint( FALSE );
        }
        break;

    case QEvent::FocusOut:
    case QEvent::FocusIn:
	widget->repaint( FALSE );
	break;

    case QEvent::Resize:
	updateRegion( widget );
	break;

    case QEvent::Move:
	if ( widget->paletteBackgroundPixmap() &&
	     widget->backgroundOrigin() != QWidget::WidgetOrigin )
	    widget->update();
	break;

    default:
        break;
    }

    return QWindowsStyle::eventFilter( o, e );
}

extern "C" __declspec(dllexport) QStyle* WINAPI createXpStyle()
{
	return new QWindowsXPStyle;
}

/**
 * DLL's entry point
 **/
int WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	return TRUE;
}

/**
 * This is to prevent the CRT from loading, thus making this a smaller
 * and faster dll.
 **/
extern "C" BOOL __stdcall _DllMainCRTStartup( HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    return DllMain( hinstDLL, fdwReason, lpvReserved );
}

#include "moc_xpstyle.cpp"

