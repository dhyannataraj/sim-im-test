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
#include <qpushbutton.h>
#include <qscrollbar.h>
#include <qbitmap.h>

#define INCLUDE_MENUITEM_DEF 1
#include <qmenudata.h>

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
            GetThemeBackgroundRegion( handle(), painter ? painter->handle() : 0, partId, stateId, &rect(), &hrgn );
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

const QPixmap *QWindowsXPStylePrivate::tabBody( QWidget *widget )
{
    if ( !tabbody ) {
        tabbody = new QPixmap( 1, 1 );
        QPainter painter( tabbody );
        XPThemeData theme( widget, &painter, "TAB", TABP_BODY, 0 );
        SIZE sz;
        GetThemePartSize( theme.handle(), painter.handle(), TABP_BODY, 0, 0, TS_TRUE, &sz );
        painter.end();
        tabbody->resize( sz.cx, sz.cy );
        painter.begin( tabbody );
        theme.rec = QRect( 0, 0, sz.cx, sz.cy );
        theme.drawBackground();
        painter.end();
    }
    return tabbody;
}

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
        widget->setMouseTracking( TRUE );
        widget->setBackgroundOrigin( QWidget::ParentOrigin );
        widget->setAutoMask(true);
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
        widget->setBackgroundPixmap( *d->tabBody( widget ) );
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
        widget->setBackgroundPixmap( QPixmap() );
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

class QMyTabBar : public QTabBar
{
public:
    virtual QTab* selectTab ( const QPoint & p ) const;
    int indexOf(QTab *t);
private:
    QMyTabBar();
};

int QMyTabBar::indexOf(QTab *t)
{
    QList<QTab> *l = tabList();
    if (l == NULL) return -1;
    int res = 0;
    for (QTab *lt = l->first(); lt; lt = l->next(), res++){
        if (lt == t) return res;
    }
    return -1;
}

QTab *QMyTabBar::selectTab(const QPoint &p) const
{
    return QTabBar::selectTab(p);
};

class QMyHeader : public QHeader
{
public:
    QRect sectionRect ( int index );
private:
    QMyHeader();
};

QRect QMyHeader::sectionRect(int index)
{
    return QHeader::sRect(index);
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
                QMyTabBar* bar = (QMyTabBar*)o;
                QTab * t = bar->selectTab( me->pos() );
                if ( d->hotTab != t ) {
                    d->hotTab = t;
                    widget->repaint( FALSE );
                }
            } else if ( o->inherits( "QHeader" ) ) {
                QMyHeader *header = (QMyHeader*)o;
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

                int x = 0, y = 0, wi = widget->width(), he = widget->height();
                int xpos = x;
                xpos += wi - 2 - 16;
                QRect rect(xpos, y+2, 16, he-4);

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
        if (widget->inherits("QCheckBox") || widget->inherits("QRadioButton"))
            d->currentWidget = widget;
        widget->repaint( TRUE );
        break;

    case QEvent::Leave:
        if ( !widget->isActiveWindow() )
            break;
        if ( widget == d->hotWidget) {
            d->hotWidget = 0;
            d->hotHeader = QRect();
            d->hotTab = 0;
            if (widget->inherits("QCheckBox") || widget->inherits("QRadioButton"))
                d->currentWidget = widget;
            widget->repaint( FALSE );
        }
        break;

    case QEvent::FocusOut:
    case QEvent::FocusIn:
        if (widget->inherits("QCheckBox") || widget->inherits("QRadioButton"))
            d->currentWidget = widget;
        widget->repaint( FALSE );
        break;

    case QEvent::Resize:
        updateRegion( widget );
        break;

    case QEvent::Move:
        if ( widget->backgroundPixmap() &&
                widget->backgroundOrigin() != QWidget::WidgetOrigin )
            widget->update();
        break;

    default:
        break;
    }

    return QWindowsStyle::eventFilter( o, e );
}

void QWindowsXPStyle::drawButton( QPainter *p, int x, int y, int w, int h,
                                  const QColorGroup &g, bool sunken, const QBrush *fill)
{
    XPThemeData theme( 0, p, "BUTTON", BP_PUSHBUTTON,
                       sunken ? PBS_PRESSED : PBS_NORMAL, QRect(x, y, w, h));
    if ( !theme.isValid() ) {
        QWindowsStyle::drawButton(p, x, y, w, h, g, sunken, fill);
        return;
    }
    theme.drawBackground();
}

void QWindowsXPStyle::drawBevelButton( QPainter *p, int x, int y, int w, int h,
                                       const QColorGroup &g, bool sunken, const QBrush *fill)
{
    drawButton(p, x, y, w, h, g, sunken, fill);
}

void QWindowsXPStyle::drawButtonMask( QPainter *p, int x, int y, int w, int h)
{
    QRect rect = QRect(x, y, w+1, h+1);

    XPThemeData theme(0, p, "BUTTON", BP_PUSHBUTTON, 0, rect);
    HRGN rgn = theme.mask();

    if ( !rgn ) {
        QWindowsStyle::drawButtonMask( p, x, y, w, h);
        return;
    }

    p->save();
    p->setBrush( color1 );
    PaintRgn( p->handle(), rgn );
    p->restore();
}

void QWindowsXPStyle::drawToolButton( QPainter *p, int x, int y, int w, int h,
                                      const QColorGroup &g, bool sunken, const QBrush *fill)
{
    XPThemeData theme( 0, p, "TOOLBAR", TP_BUTTON,
                       sunken ? TS_PRESSED : TS_NORMAL, QRect(x, y, w, h));
    if ( !theme.isValid() ) {
        QWindowsStyle::drawToolButton(p, x, y, w, h, g, sunken, fill);
        return;
    }
    theme.drawBackground();
}

void QWindowsXPStyle::drawPushButton( QPushButton* btn, QPainter *p)
{
    int stateId = PBS_NORMAL;
    if (!btn->isEnabled()){
        stateId = PBS_DISABLED;
    }else if ((btn->isToggleButton() && btn->isOn()) || btn->isDown()){
        stateId = PBS_PRESSED;
    }else if (btn == d->hotWidget){
        stateId = PBS_HOT;
    }else if (btn->isDefault()){
        stateId = PBS_DEFAULTED;
    }

    XPThemeData theme( 0, p, "BUTTON", BP_PUSHBUTTON,
                       stateId, QRect(btn->rect()));
    if ( !theme.isValid() ) {
        QWindowsStyle::drawPushButton(btn, p);
        return;
    }
    theme.drawBackground();
}

class QMyScrollBar : public QScrollBar
{
public:
    int	sliderStart() const { return QScrollBar::sliderStart(); }
private:
    QMyScrollBar();
};

void QWindowsXPStyle::drawScrollBarControls( QPainter *p,  const QScrollBar *sbar,
        int sliderStart, uint controls,
        uint activeControl )
{
    QMyScrollBar *bar = (QMyScrollBar*)sbar;
    int sliderstart = 0;

    int s = bar->orientation() == Qt::Horizontal ?
            QApplication::globalStrut().height()
            : QApplication::globalStrut().width();
    int sbextent = QMAX( 16, s );
    int maxlen = ((bar->orientation() == Qt::Horizontal) ?
                  bar->width() : bar->height()) - (sbextent * 2);
    int sliderlen;

    sliderstart = bar->sliderStart();

    // calculate slider length
    if (bar->maxValue() != bar->minValue()) {
        uint range = bar->maxValue() - bar->minValue();
        sliderlen = (bar->pageStep() * maxlen) /
                    (range + bar->pageStep());

        int slidermin = 9;
        if ( sliderlen < slidermin || range > INT_MAX / 2 )
            sliderlen = slidermin;
        if ( sliderlen > maxlen )
            sliderlen = maxlen;
    } else
        sliderlen = maxlen;

    int partId = 0;
    int stateId = 0;
    XPThemeData theme( bar, p, "SCROLLBAR" );
    bool maxedOut = ( bar->maxValue() == bar->minValue() );
    if ( controls & AddLine ) {
        QRect rc;
        if (bar->orientation() == Qt::Horizontal)
            rc = QRect(bar->width() - sbextent, 0, sbextent, sbextent);
        else
            rc = QRect(0, bar->height() - sbextent, sbextent, sbextent);
        theme.rec = rc;
        partId = SBP_ARROWBTN;
        if ( maxedOut )
            stateId = ABS_DOWNDISABLED;
        else if ( activeControl == AddLine )
            stateId = ABS_DOWNPRESSED;
        else if ( theme.rec.contains( d->hotSpot ) )
            stateId = ABS_DOWNHOT;
        else
            stateId = ABS_DOWNNORMAL;
        if ( bar->orientation() == Qt::Horizontal )
            stateId += 8;
        theme.drawBackground( partId, stateId );
    }
    if ( controls & SubLine ) {
        QRect rc(0, 0, sbextent, sbextent);
        theme.rec = rc;
        partId = SBP_ARROWBTN;
        if ( maxedOut )
            stateId = ABS_UPDISABLED;
        else if ( activeControl == SubLine )
            stateId = ABS_UPPRESSED;
        else if ( theme.rec.contains( d->hotSpot ) )
            stateId = ABS_UPHOT;
        else
            stateId = ABS_UPNORMAL;
        if ( bar->orientation() == Qt::Horizontal )
            stateId += 8;
        theme.drawBackground( partId, stateId );
    }
    if ( maxedOut ) {
        if (bar->orientation() == Qt::Horizontal){
            theme.rec = QRect(sliderstart, 0, sliderlen, sbextent);
            theme.rec = theme.rec.unite(QRect(sbextent, 0, sliderstart - sbextent, sbextent));
            theme.rec = theme.rec.unite(QRect(sliderstart + sliderlen, 0, maxlen - sliderstart - sliderlen + sbextent, sbextent));
            partId = SBP_LOWERTRACKHORZ;
        }else{
            theme.rec = QRect(0, sliderstart, sbextent, sliderlen);
            theme.rec = theme.rec.unite(QRect(0, sbextent, sbextent, sliderstart - sbextent));
            theme.rec = theme.rec.unite(QRect(0, sliderstart + sliderlen, sbextent, maxlen - sliderstart - sliderlen + sbextent));
            partId = SBP_LOWERTRACKVERT;
        }
        stateId = SCRBS_DISABLED;
        theme.drawBackground( partId, stateId );
    } else {
        if ( controls & AddPage ) {
            if (bar->orientation() == Qt::Horizontal){
                theme.rec = QRect(sliderstart + sliderlen, 0,
                                  maxlen - sliderstart - sliderlen + sbextent, sbextent);
                partId = SBP_LOWERTRACKHORZ;
            }else{
                theme.rec = QRect(0, sliderstart + sliderlen,
                                  sbextent, maxlen - sliderstart - sliderlen + sbextent);
                partId = SBP_LOWERTRACKVERT;
            }
            if ( !bar->isEnabled() )
                stateId = SCRBS_DISABLED;
            else if ( activeControl == AddPage )
                stateId = SCRBS_PRESSED;
            else if ( theme.rec.contains( d->hotSpot ) )
                stateId = SCRBS_HOT;
            else
                stateId = SCRBS_NORMAL;
            theme.drawBackground( partId, stateId );
        }
        if ( controls & SubPage ) {
            if (bar->orientation() == Qt::Horizontal){
                theme.rec = QRect(sbextent, 0, sliderstart - sbextent, sbextent);
                partId = SBP_UPPERTRACKHORZ;
            }else{
                theme.rec = QRect(0, sbextent, sbextent, sliderstart - sbextent);
                partId = SBP_UPPERTRACKVERT;
            }
            if ( !bar->isEnabled() )
                stateId = SCRBS_DISABLED;
            else if ( activeControl == AddPage )
                stateId = SCRBS_PRESSED;
            else if ( theme.rec.contains( d->hotSpot ) )
                stateId = SCRBS_HOT;
            else
                stateId = SCRBS_NORMAL;
            theme.drawBackground( partId, stateId );
        }
        if ( controls & Slider ) {
            if (bar->orientation() == Qt::Horizontal)
                theme.rec = QRect(sliderstart, 0, sliderlen, sbextent);
            else
                theme.rec = QRect(0, sliderstart, sbextent, sliderlen);
            if ( !bar->isEnabled() )
                stateId = SCRBS_DISABLED;
            else if ( activeControl == Slider )
                stateId = SCRBS_PRESSED;
            else if ( theme.rec.contains( d->hotSpot ) )
                stateId = SCRBS_HOT;
            else
                stateId = SCRBS_NORMAL;
            const int swidth = theme.rec.width();
            const int sheight = theme.rec.height();
            theme.drawBackground((bar->orientation() == Qt::Horizontal) ? SBP_THUMBBTNHORZ : SBP_THUMBBTNVERT, stateId );
            QRect gr;
            if ( bar->orientation() == Qt::Horizontal ) {
                gr.setLeft( theme.rec.left() + swidth/2 - 5 );
                gr.setRight( gr.left() + 10 );
                gr.setTop( theme.rec.top() + sheight/2 - 3 );
                gr.setBottom( gr.top() + 6 );
            } else {
                gr.setLeft( theme.rec.left() + swidth/2 - 3 );
                gr.setRight( gr.left() + 6 );
                gr.setTop( theme.rec.top() + sheight/2 - 5 );
                gr.setBottom( gr.top() + 10 );
            }
            theme.rec = gr;
            theme.drawBackground( (bar->orientation() == Qt::Horizontal ) ? SBP_GRIPPERHORZ : SBP_GRIPPERVERT, 1 );
        }
    }
}

QSize QWindowsXPStyle::exclusiveIndicatorSize() const
{
    XPThemeData theme( 0, 0, "BUTTON", BP_RADIOBUTTON, RBS_UNCHECKEDNORMAL );
    if ( theme.isValid() ) {
        SIZE size;
        GetThemePartSize( theme.handle(), NULL, theme.partId, theme.stateId, 0, TS_TRUE, &size );
        return QSize(size.cx+2, size.cy+2);
    }
    return QWindowsStyle::indicatorSize();
}

void QWindowsXPStyle::drawExclusiveIndicator( QPainter* p, int x, int y, int w, int h,
        const QColorGroup &g, bool on, bool down, bool enabled)
{
    int stateId = 0;
    if ( !enabled )
        stateId = RBS_UNCHECKEDDISABLED;
    else if ( down )
        stateId = RBS_UNCHECKEDPRESSED;
    else if ( d->currentWidget && (d->currentWidget == d->hotWidget) && d->currentWidget->inherits("QRadioButton") )
        stateId = RBS_UNCHECKEDHOT;
    else
        stateId = RBS_UNCHECKEDNORMAL;
    if ( on )
        stateId += RBS_CHECKEDNORMAL-1;

    XPThemeData theme( 0, p, "BUTTON", BP_RADIOBUTTON, stateId, QRect(x, y, w, h));
    if ( !theme.isValid() ) {
        QWindowsStyle::drawExclusiveIndicator( p, x, y, w, h, g, on, down, enabled);
        return;
    }
    theme.drawBackground();
}

void QWindowsXPStyle::drawExclusiveIndicatorMask( QPainter *p, int x, int y, int w, int h, bool on)
{
    QRect rect = QRect(x, y, w+1, h+1);
    XPThemeData theme(0, p, "BUTTON", BP_RADIOBUTTON, RBS_UNCHECKEDNORMAL, rect);
    HRGN rgn = theme.mask();

    if ( !rgn ) {
        p->save();
        p->setBrush( color1 );
        p->setPen( QPen( NoPen ) );
        p->drawEllipse( x, y, w, h );
        p->restore();
        return;
    }
    p->save();
    p->setBrush( color1 );
    PaintRgn( p->handle(), rgn );
    p->restore();
}

QSize QWindowsXPStyle::indicatorSize() const
{
    XPThemeData theme( 0, 0, "BUTTON", BP_CHECKBOX, CBS_UNCHECKEDNORMAL);
    if ( theme.isValid() ) {
        SIZE size;
        GetThemePartSize( theme.handle(), NULL, theme.partId, theme.stateId, 0, TS_TRUE, &size );
        return QSize(size.cx+2, size.cy+2);
    }
    return QWindowsStyle::indicatorSize();
}

void QWindowsXPStyle::drawIndicator( QPainter* p, int x, int y, int w, int h, const QColorGroup &g,
                                     int state, bool down, bool enabled)
{
    int stateId = 0;
    if ( !enabled )
        stateId = CBS_UNCHECKEDDISABLED;
    else if ( down )
        stateId = CBS_UNCHECKEDPRESSED;
    else if ( d->currentWidget && (d->currentWidget == d->hotWidget) && d->currentWidget->inherits("QCheckButton") )
        stateId = CBS_UNCHECKEDHOT;
    else
        stateId = CBS_UNCHECKEDNORMAL;
    if ( state == QButton::On )
        stateId += CBS_CHECKEDNORMAL-1;
    else if ( state == QButton::NoChange )
        stateId += CBS_MIXEDNORMAL-1;

    XPThemeData theme( 0, p, "BUTTON", BP_CHECKBOX, stateId, QRect(x, y, w, h));
    if ( !theme.isValid() ) {
        QWindowsStyle::drawIndicator( p, x, y, w, h, g, state, down, enabled);
        return;
    }
    theme.drawBackground();
}

void QWindowsXPStyle::drawIndicatorMask( QPainter *p, int x, int y, int w, int h, int state)
{
    QRect rect = QRect(x, y, w+1, h+1);
    XPThemeData theme(0, p, "BUTTON", BP_CHECKBOX, CBS_UNCHECKEDNORMAL, rect);
    HRGN rgn = theme.mask();

    if ( !rgn ) {
        QWindowsStyle::drawButtonMask( p, x, y, w, h);
        return;
    }
    p->save();
    p->setBrush( color1 );
    PaintRgn( p->handle(), rgn );
    p->restore();
}

void QWindowsXPStyle::drawComboButton( QPainter *p, int x, int y, int w, int h,
                                       const QColorGroup &g, bool sunken, bool editable,
                                       bool enabled, const QBrush *fill)
{
    int partId = 0;
    int stateId = 0;
    partId = EP_EDITTEXT;
    if ( !enabled )
        stateId = ETS_DISABLED;
    else
        stateId = ETS_NORMAL;
    XPThemeData theme1( 0, p, "EDIT", partId, stateId, QRect(x, y, w, h) );
    theme1.drawBackground();
    XPThemeData theme( 0, p, "COMBOBOX" );
    int xpos = x;
    xpos += w - 2 - 16;
    theme.rec = QRect(xpos, y+2, 16, h-4);
    partId = CP_DROPDOWNBUTTON;

    if ( !enabled )
        stateId = CBXS_DISABLED;
    else if ( sunken )
        stateId = CBXS_PRESSED;
    //		else if ( flags & Style_MouseOver && theme.rec.contains( d->hotSpot ) )
    //		    stateId = CBXS_HOT;
    else
        stateId = CBXS_NORMAL;

    theme.drawBackground( partId, stateId );
}

void QWindowsXPStyle::drawPanel( QPainter *p, int x, int y, int w, int h,
                                 const QColorGroup &cg, bool sunken, int lineWidth, const QBrush *fill)
{
    //	if (d->currentWidget && d->currentWidget->inherits("QLineEdit")){
    XPThemeData theme( 0, p, "EDIT", EP_EDITTEXT, ETS_NORMAL, QRect(x, y, w, h));
    if ( theme.isValid() ) {
        theme.drawBackground();
        return;
    }
    //	}
    QWindowsStyle::drawPanel(p, x, y, w, h, cg, sunken, lineWidth, fill);
}

void QWindowsXPStyle::drawPopupPanel( QPainter *p, int x, int y, int w, int h,
                                      const QColorGroup &,  int lineWidth, const QBrush *fill)
{
}

void QWindowsXPStyle::drawTab( QPainter *p, const QTabBar *tbar, QTab *t, bool selected )
{
    if (tbar->shape() != QTabBar::RoundedAbove){
        QWindowsStyle::drawTab( p, tbar, t, selected );
        return;
    }
    QRect rect( t->r );
    QMyTabBar *bar = (QMyTabBar*)tbar;
    int partId = 0;
    int stateId = 0;
    int idx = bar->indexOf( t );
    int aidx = bar->indexOf( bar->tab(bar->currentTab()) );
    if ( idx == 0 )
        partId = TABP_TABITEMLEFTEDGE;
    else if ( idx == bar->count()-1 )
        partId = TABP_TABITEMRIGHTEDGE;
    else
        partId = TABP_TABITEM;

    if ( !bar->isEnabled())
        stateId = TIS_DISABLED;
    //	    else if ( flags & Style_HasFocus )
    //		stateId = TIS_FOCUSED;
    else if ( t == bar->tab(bar->currentTab() ))
        stateId = TIS_SELECTED;
    else if ( t && d->hotTab == t )
        stateId = TIS_HOT;
    else
        stateId = TIS_NORMAL;
    //	    if ( (flags & Style_Selected) || (flags & Style_HasFocus) ) {
    if ( t == bar->tab(bar->currentTab() )){
        //		rect = QRect( rect.x(), rect.y(), rect.width() + 1, rect.height() + 1 );
    } else {
        rect = QRect( rect.x(), rect.y() + 2, rect.width(), rect.height() );
        if ( idx != aidx+1 )
            rect = QRect( rect.x() + 1, rect.y(), rect.width(), rect.height() );
        if ( idx != aidx-1 )
            rect = QRect( rect.x(), rect.y(), rect.width()-1, rect.height() );
    }
    XPThemeData theme( bar, p, "TAB", partId, stateId, rect );
    if ( !theme.isValid() ) {
        QWindowsStyle::drawTab( p, bar, t, selected );
        return;
    }

    theme.drawBackground();

}

void QWindowsXPStyle::drawTabMask( QPainter*, const QTabBar*, QTab*, bool selected )
{
}

static const unsigned bitmapWidth = 22;
static const unsigned itemVMargin = 4;
static const unsigned rightBorder = 12;
static const unsigned itemFrame = 0;
static const unsigned itemHMargin = 3;
static const unsigned shadowWidth = 3;
static const unsigned arrowHMargin = 6;

void QWindowsXPStyle::drawPopupMenuItem( QPainter* p, bool checkable,
        int maxpmw, int tab, QMenuItem* mi,
        const QPalette& pal,
        bool act, bool enabled,
        int x, int y, int w, int h)
{
    int checkcol = 22;
    QRect r(x, y, w, h);
    QColorGroup cg = pal.active();
    if (act){
        p->setPen(cg.highlight());
        p->fillRect(x, y, w, h, enabled ? colorSel(cg) : colorMenu(pal.disabled()));
        p->drawLine(x+1, y, x+w-2, y);
        p->drawLine(x+1, y+h-1, x+w-2, y+h-1);
        p->drawLine(x, y+1, x, y+h-2);
        p->drawLine(x+w-1, y+1, x+w-1, y+h-2);
    }else{
        drawMenuBackground(p, r, cg);
    }

    int xpos = x;
    int xm = itemFrame + checkcol + itemHMargin;
    if (mi->isSeparator()) {
        p->setPen(cg.mid());
        if ((int)(x + bitmapWidth + 2) < (int)(x + w - 2))
            p->drawLine(x + bitmapWidth + 2, y + h / 2,
                        x + w - 2, y + h / 2);
        return;
    }

    if (mi->iconSet()) {
        p->save();
        QIconSet::Mode mode =
            enabled ? QIconSet::Normal : QIconSet::Disabled;
        QPixmap pixmap = mi->iconSet()->pixmap(QIconSet::Small, mode);
        int pixw = pixmap.width();
        int pixh = pixmap.height();

        QRect cr(xpos, y, 22, h);
        QRect pmr(0, 0, pixw, pixh);
        if (checkable && mi->isChecked()){
            p->setPen(darkXp(enabled ? colorSel(cg) : colorMenu(pal.disabled())));
            p->drawLine(cr.x(), cr.y(), cr.x() + cr.width()-1, cr.y());
            p->drawLine(cr.x(), cr.y(), cr.x(), cr.y() + cr.height());
            p->setPen(colorMenu(cg));
            p->drawLine(cr.x(), cr.y()+cr.height()-1,
                        cr.x()+cr.width()-1, cr.y()+cr.height()-1);
            cr.moveBy(1, 1);
        }
        pmr.moveCenter(cr.center());
        QPoint picPoint = pmr.topLeft();
        if (act && enabled){
            const QBitmap *mask = pixmap.mask();
            if (mask){
                QPixmap maskPict = *mask;
                maskPict.setMask(*mask);
                p->setBrush(darkXp(colorSel(cg)));
                picPoint += QPoint(1, 1);
                p->drawPixmap(picPoint, maskPict);
                picPoint -= QPoint(2, 2);
            }
        }
        p->setPen(cg.highlightedText());
        p->drawPixmap(picPoint, pixmap);
        p->restore();

    }else if (checkable && mi->isChecked()){
        p->setPen(cg.buttonText());
        drawCheckMark(p, xpos, y, bitmapWidth, h, cg, act, !enabled);
    }

    xpos += xm;
    if (mi->custom()) {
        int m = itemVMargin;
        p->save();
        p->setPen(cg.text());
        mi->custom()->paint(p, cg, act, enabled,
                            x+xm, y+m, w-xm-tab+1, h-2*m);
        p->restore();
        return;
    }else{
        QString s = mi->text();
        if(!s.isNull()) {
            int t = s.find('\t');
            int m = itemVMargin;

            const int text_flags =
                AlignVCenter | ShowPrefix | DontClip | SingleLine;

            p->setPen(cg.text());
            if (t >= 0) {
                int xp;
                xp = x + w - tab - rightBorder - itemHMargin - itemFrame + 1;
                p->drawText(xp, y+m, tab, h-2*m, text_flags, s.mid(t+1));
                s = s.left(t);
            }
            p->drawText(xpos, y+m, w-xm-tab+1, h-2*m, text_flags, s);
        }
    }
    if (mi->popup()) {
        int dim = (h - 2 * itemFrame) / 2;
        xpos = x+w - arrowHMargin - itemFrame - dim;
        p->setPen(cg.text());
        drawArrow(p, RightArrow, false, xpos, y + h / 4, dim, dim, cg, enabled);
    }
}

QColor QWindowsXPStyle::mix(const QColor &c1, const QColor &c2, int k) const
{
    int r1, g1, b1;
    int r2, g2, b2;
    c1.rgb(&r1, &g1, &b1);
    c2.rgb(&r2, &g2, &b2);
    return QColor(
               ((r1 * k) + (r2 * (100-k)))/100,
               ((g1 * k) + (g2 * (100-k)))/100,
               ((b1 * k) + (b2 * (100-k)))/100);
}

QColor QWindowsXPStyle::colorMenu(const QColorGroup &cg) const
{
    return mix(cg.background(), cg.button(), 86).light(110);
}

QColor QWindowsXPStyle::colorBitmap(const QColorGroup &cg) const
{
    return mix(cg.background(), cg.button(), 14).dark(120);
}

QColor QWindowsXPStyle::colorSel(const QColorGroup &cg) const
{
    return mix(cg.background(), cg.highlight(), 70);
}

QColor QWindowsXPStyle::darkXp(const QColor &c) const
{
    int r, g, b;
    c.rgb(&r, &g, &b);
    b = QMIN(b + 55, 255);
    return mix(QColor(r, g, b), QColor(0, 0, 0), 70);
}


void QWindowsXPStyle::drawMenuBackground(QPainter *p, const QRect &rc, const QColorGroup &cg) const
{
    p->fillRect(rc, colorMenu(cg));
    QRect rcBitmap(0, rc.y(), bitmapWidth, rc.height());
    p->fillRect(rcBitmap, colorBitmap(cg));
}

void QWindowsXPStyle::activeTabChanged()
{
    const QObject *s = sender();
    if ( !s || !s->inherits( "QTabBar" ) )
        return;

    ((QWidget *)s)->repaint( FALSE );
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

