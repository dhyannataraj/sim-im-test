/****************************************************************************
** $Id: qtooltip.h,v 1.2 2002-12-15 01:59:04 shutoff Exp $
**
** Definition of Tool Tips (or Balloon Help) for any widget or rectangle
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef QTOOLTIP_H
#define QTOOLTIP_H

#ifndef QT_H
#include "qwidget.h"
#include "qtimer.h"
#include "qlabel.h"
#include "qptrdict.h"
#include "qguardedptr.h"
#endif // QT_H

#ifndef QT_NO_TOOLTIP

class MyQTipManager;
class QLabel;

class MyQToolTipGroup: public QObject
{
    Q_OBJECT
    Q_PROPERTY( bool delay READ delay WRITE setDelay )
    Q_PROPERTY( bool enabled READ enabled WRITE setEnabled )

public:
    MyQToolTipGroup( QObject *parent, const char *name = 0 );
    ~MyQToolTipGroup();

    bool delay() const;
    bool enabled() const;

public slots:
    void setDelay( bool );
    void setEnabled( bool );

signals:
    void showTip( const QString &);
    void removeTip();

private:
uint del:1;
uint ena:1;

    friend class QTipManager;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QToolTipGroup( const QToolTipGroup & );
    QToolTipGroup& operator=( const QToolTipGroup & );
#endif
};


class MyQToolTip: public Qt
{
public:
    MyQToolTip( QWidget *, MyQToolTipGroup * = 0 );

    static void add( QWidget *, const QString &);
    static void add( QWidget *, const QString &,
                     MyQToolTipGroup *, const QString& );
    static void remove( QWidget * );

    static void add( QWidget *, const QRect &, const QString &);
    static void add( QWidget *, const QRect &, const QString &,
                     MyQToolTipGroup *, const QString& );
    static void remove( QWidget *, const QRect & );

    static void hide();

    static QFont    font();
    static void	    setFont( const QFont & );
    static QPalette palette();
    static void	    setPalette( const QPalette & );

    static void	    setEnabled( bool );
    static bool	    enabled();

protected:
    virtual void maybeTip( const QPoint & ) = 0;
    void    tip( const QRect &, const QString &);
    void    tip( const QRect &, const QString& , const QString &);
    void    clear();

public:
    QWidget	  *parentWidget() const { return p; }
    MyQToolTipGroup *group()	  const { return g; }

private:
    void    tip( const QRect &, const QRect &, const QString &);
    QWidget	    *p;
    MyQToolTipGroup   *g;
    static QFont    *ttFont;
    static QPalette *ttPalette;

    static void initialize();
    static void cleanup();

    friend class QTipManager;
};

// Internal class - don't touch

class QTipLabel : public QLabel
{
    Q_OBJECT
public:
QTipLabel(const QString& text) : QLabel( 0, "toolTipTip",
            WStyle_StaysOnTop +
            WStyle_Customize + WStyle_NoBorder + WStyle_Tool )
    {
        setMargin(1);
        setIndent(0);
        setAutoMask( FALSE );
        setFrameStyle( QFrame::Plain | QFrame::Box );
        setLineWidth( 1 );
        setAlignment( AlignLeft | AlignTop );
        polish();
        setText(text);
        adjustSize();
    }
};

// Internal class - don't touch

class QTipManager : public QObject
{
    Q_OBJECT
public:
    QTipManager();
    ~QTipManager();

    struct Tip
    {
        QRect		rect;
        QString		text;
        QString	        groupText;
        MyQToolTipGroup  *group;
        MyQToolTip       *tip;
        bool	        autoDelete;
        QRect 		geometry;
        Tip	       *next;
    };

    bool    eventFilter( QObject * o, QEvent * e );
    void    add( const QRect &gm, QWidget *, const QRect &, const QString& ,
                 MyQToolTipGroup *, const QString& , MyQToolTip *, bool );
    void    add( QWidget *, const QRect &, const QString& ,
                 MyQToolTipGroup *, const QString& , MyQToolTip *, bool );
    void    remove( QWidget *, const QRect & );
    void    remove( QWidget * );

    void    removeFromGroup( MyQToolTipGroup * );

    void    hideTipAndSleep();

public slots:
    void    hideTip();

private slots:
    void    labelDestroyed();
    void    clientWidgetDestroyed();
    void    showTip();
    void    allowAnimation();

private:
    QTimer  wakeUp;
    QTimer  fallAsleep;

    QPtrDict<Tip> *tips;
    QLabel *label;
    QPoint pos;
    QGuardedPtr<QWidget> widget;
    Tip *currentTip;
    Tip *previousTip;
    bool preventAnimation;
    bool isApplicationFilter;
    QTimer *removeTimer;
};


#endif // QT_NO_TOOLTIP

#endif // QTOOLTIP_H
