/****************************************************************************
** $Id: qstylesheet.h,v 1.3 2003-10-05 00:45:31 shutoff Exp $
**
** Definition of the QStyleSheet class
**
** Created : 990101
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
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

#ifndef QSTYLESHEET_H
#define QSTYLESHEET_H

#ifndef QT_H
#include "qstring.h"
#include "qvaluelist.h"
#include "qdict.h"
#include "qobject.h"
#include "qt3stuff.h"
#include "qmap.h"
#endif // QT_H

#if QT_VERSION < 300

#include "simapi.h"

#ifndef QT_NO_RICHTEXT

namespace Qt3
{

class QStyleSheet;
class QTextDocument;
class QStyleSheetItemData;

class UI_EXPORT QStyleSheetItem : public Qt
{
public:
    QStyleSheetItem( QStyleSheet* parent, const QString& name );
    QStyleSheetItem( const QStyleSheetItem & );
    ~QStyleSheetItem();

    QString name() const;

    QStyleSheet* styleSheet();
    const QStyleSheet* styleSheet() const;

    enum AdditionalStyleValues { Undefined  = - 1};

    enum DisplayMode {
        DisplayBlock,
        DisplayInline,
        DisplayListItem,
        DisplayNone
    };

    DisplayMode displayMode() const;
    void setDisplayMode(DisplayMode m);

    int alignment() const;
    void setAlignment( int f);

    enum VerticalAlignment {
        VAlignBaseline,
        VAlignSub,
        VAlignSuper
    };

    VerticalAlignment verticalAlignment() const;
    void setVerticalAlignment( VerticalAlignment valign );

    int fontWeight() const;
    void setFontWeight(int w);

    int logicalFontSize() const;
    void setLogicalFontSize(int s);

    int logicalFontSizeStep() const;
    void setLogicalFontSizeStep( int s );

    int fontSize() const;
    void setFontSize(int s);

    QString fontFamily() const;
    void setFontFamily( const QString& );

    int numberOfColumns() const;
    void setNumberOfColumns(int ncols);

    QColor color() const;
    void setColor( const QColor &);

    bool fontItalic() const;
    void setFontItalic( bool );
    bool definesFontItalic() const;

    bool fontUnderline() const;
    void setFontUnderline( bool );
    bool definesFontUnderline() const;

    bool isAnchor() const;
    void setAnchor(bool anc);

    enum WhiteSpaceMode { WhiteSpaceNormal, WhiteSpacePre, WhiteSpaceNoWrap };
    WhiteSpaceMode whiteSpaceMode() const;
    void setWhiteSpaceMode(WhiteSpaceMode m);

    enum Margin {
        MarginLeft,
        MarginRight,
        MarginTop,
        MarginBottom,
        MarginFirstLine,
        MarginAll,
        MarginVertical,
        MarginHorizontal
    };

    int margin( Margin m) const;
    void setMargin( Margin, int);

    enum ListStyle {
        ListDisc,
        ListCircle,
        ListSquare,
        ListDecimal,
        ListLowerAlpha,
        ListUpperAlpha
    };

    ListStyle listStyle() const;
    void setListStyle( ListStyle );

    QString contexts() const;
    void setContexts( const QString& );
    bool allowedInContext( const QStyleSheetItem* ) const;

    bool selfNesting() const;
    void setSelfNesting( bool );

    void setLineSpacing( int ls );
    int lineSpacing() const;

private:
    void init();
    QStyleSheetItemData* d;
};

class QTextCustomItem;

class UI_EXPORT QStyleSheet : public QObject
{
    Q_OBJECT
public:
    QStyleSheet( QObject *parent=0, const char *name=0 );
    virtual ~QStyleSheet();

    static QStyleSheet* defaultSheet();
    static void setDefaultSheet( QStyleSheet* );


    QStyleSheetItem* item( const QString& name);
    const QStyleSheetItem* item( const QString& name) const;

    void insert( QStyleSheetItem* item);

    virtual QTextCustomItem* tag( const QString& name,
                                  const QMap<QString, QString> &attr,
                                  const QString& context,
                                  const QMimeSourceFactory& factory,
                                  bool emptyTag, QTextDocument *doc ) const;

    static QString escape( const QString& );
    static QString convertFromPlainText( const QString&, QStyleSheetItem::WhiteSpaceMode mode = QStyleSheetItem::WhiteSpacePre );
    static bool mightBeRichText( const QString& );

    virtual void scaleFont( QFont& font, int logicalSize ) const;

    virtual void error( const QString& ) const;

private:
    void init();
    QDict<QStyleSheetItem> *pStyles;
    QStyleSheetItem* nullstyle;
};

}

#endif // QT_NO_RICHTEXT

#endif

#endif // QSTYLESHEET_H
