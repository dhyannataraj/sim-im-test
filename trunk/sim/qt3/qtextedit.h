/****************************************************************************
** $Id: qtextedit.h,v 1.2 2002-07-07 15:32:07 shutoff Exp $
**
** Definition of the QTextEdit class
**
** Created : 990101
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

#ifndef QTEXTEDIT_H
#define QTEXTEDIT_H

#ifndef QT_H
#include "qscrollview.h"
#include "qstylesheet.h"
#include "qvaluelist.h"
#include "qvector.h"
#include "qt3stuff.h"
#endif // QT_H

#if QT_VERSION < 300

class QKeyEvent;
class QResizeEvent;
class QMouseEvent;
class QTimer;
class QColor;
class QFont;
class QPopupMenu;
class QPainter;

class QTextEdit;
class QTextBrowser;

namespace Qt3 {

#ifndef QT_NO_TEXTEDIT
class QTextDocument;
class QTextCursor;
class QTextString;
class QTextCommand;
class QTextParag;
class QTextFormat;
class QTextString;
struct QUndoRedoInfoPrivate;
class QTextEditPrivate;

}

using namespace Qt3;

class QTextEdit : public QScrollView
{
    friend class QTextBrowser;

    Q_OBJECT
    Q_ENUMS( WordWrap WrapPolicy )
    Q_PROPERTY( TextFormat textFormat READ textFormat WRITE setTextFormat )
    Q_PROPERTY( QString text READ text WRITE setText )
    Q_PROPERTY( QBrush paper READ paper WRITE setPaper )
    Q_PROPERTY( bool linkUnderline READ linkUnderline WRITE setLinkUnderline )
    Q_PROPERTY( QString documentTitle READ documentTitle )
    Q_PROPERTY( int length READ length )
    Q_PROPERTY( WordWrap wordWrap READ wordWrap WRITE setWordWrap )
    Q_PROPERTY( int wrapColumnOrWidth READ wrapColumnOrWidth WRITE setWrapColumnOrWidth )
    Q_PROPERTY( WrapPolicy wrapPolicy READ wrapPolicy WRITE setWrapPolicy )
    Q_PROPERTY( bool hasSelectedText READ hasSelectedText )
    Q_PROPERTY( QString selectedText READ selectedText )
    Q_PROPERTY( int undoDepth READ undoDepth WRITE setUndoDepth )
    Q_PROPERTY( bool overwriteMode READ isOverwriteMode WRITE setOverwriteMode )
    Q_PROPERTY( bool modified READ isModified WRITE setModified DESIGNABLE false )
    Q_PROPERTY( bool readOnly READ isReadOnly WRITE setReadOnly )
    Q_PROPERTY( bool undoRedoEnabled READ isUndoRedoEnabled WRITE setUndoRedoEnabled )
    Q_PROPERTY( int tabStopWidth READ tabStopWidth WRITE setTabStopWidth )

public:
    enum WordWrap {
        NoWrap,
        WidgetWidth,
        FixedPixelWidth,
        FixedColumnWidth
    };

    enum WrapPolicy {
        AtWordBoundary,
        Anywhere,
        AtWhiteSpace = AtWordBoundary // deprecated, don't use
    };

    enum KeyboardAction {
        ActionBackspace,
        ActionDelete,
        ActionReturn,
        ActionKill
    };

    enum CursorAction {
        MoveBackward,
        MoveForward,
        MoveWordBackward,
        MoveWordForward,
        MoveUp,
        MoveDown,
        MoveLineStart,
        MoveLineEnd,
        MoveHome,
        MoveEnd,
        MovePgUp,
        MovePgDown
    };

    enum VerticalAlignment {
        AlignNormal,
        AlignSuperScript,
        AlignSubScript
    };

    QTextEdit( const QString& text, const QString& context = QString::null,
               QWidget* parent=0, const char* name=0);
    QTextEdit( QWidget* parent=0, const char* name=0 );
    virtual ~QTextEdit();
    void setPalette( const QPalette & );

    QString text() const;
    QString text( int para ) const;
    TextFormat textFormat() const;
    QString context() const;
    QString documentTitle() const;

    void getSelection( int *paraFrom, int *indexFrom,
                       int *paraTo, int *indexTo, int selNum = 0 ) const;
    virtual bool find( const QString &expr, bool cs, bool wo, bool forward = TRUE,
                       int *para = 0, int *index = 0 );

    int paragraphs() const;
    int lines() const;
    int linesOfParagraph( int para ) const;
    int lineOfChar( int para, int chr );
    int length() const;
    QRect paragraphRect( int para ) const;
    int paragraphAt( const QPoint &pos ) const;
    int charAt( const QPoint &pos, int *para ) const;
    int paragraphLength( int para ) const;

    QStyleSheet* styleSheet() const;
    QMimeSourceFactory* mimeSourceFactory() const;

    QBrush paper() const;
    bool linkUnderline() const;

    int heightForWidth( int w ) const;

    bool hasSelectedText() const;
    QString selectedText() const;
    bool isUndoAvailable() const;
    bool isRedoAvailable() const;

    WordWrap wordWrap() const;
    int wrapColumnOrWidth() const;
    WrapPolicy wrapPolicy() const;

    int tabStopWidth() const;

    QString anchorAt( const QPoint& pos );

    QSize sizeHint() const;

    bool isReadOnly() const { return readonly; }

    void getCursorPosition( int *parag, int *index ) const;

    bool isModified() const;
    bool italic() const;
    bool bold() const;
    bool underline() const;
    QString family() const;
    int pointSize() const;
    QColor color() const;
    QFont font() const;
    int alignment() const;
    int undoDepth() const;
    virtual bool getFormat( int para, int index, QFont *font, QColor *color, VerticalAlignment *verticalAlignment );
    virtual bool getParagraphFormat( int para, QFont *font, QColor *color,
                                     VerticalAlignment *verticalAlignment, int *alignment,
                                     QStyleSheetItem::DisplayMode *displayMode,
                                     QStyleSheetItem::ListStyle *listStyle,
                                     int *listDepth );
    bool isOverwriteMode() const { return overWrite; }
    QColor paragraphBackgroundColor( int para ) const;

    bool isUndoRedoEnabled() const;
    bool eventFilter( QObject *o, QEvent *e );

public slots:
    void setEnabled( bool );
    virtual void setMimeSourceFactory( QMimeSourceFactory* factory );
    virtual void setStyleSheet( QStyleSheet* styleSheet );
    virtual void scrollToAnchor( const QString& name );
    virtual void setPaper( const QBrush& pap );
    virtual void setLinkUnderline( bool );

    virtual void setWordWrap( WordWrap mode );
    virtual void setWrapColumnOrWidth( int );
    virtual void setWrapPolicy( WrapPolicy policy );

    virtual void copy();
    virtual void append( const QString& text );

    void setText( const QString &txt ) { setText( txt, QString::null ); }
    virtual void setText( const QString &txt, const QString &context );
    virtual void setTextFormat( TextFormat f );

    virtual void selectAll( bool select = TRUE );
    virtual void setTabStopWidth( int ts );
    virtual void zoomIn( int range );
    virtual void zoomIn() { zoomIn( 1 ); }
    virtual void zoomOut( int range );
    virtual void zoomOut() { zoomOut( 1 ); }
    virtual void zoomTo( int size );

    virtual void sync();
    virtual void setReadOnly( bool b );

    virtual void undo();
    virtual void redo();
    virtual void cut();
    virtual void paste();
#ifndef QT_NO_CLIPBOARD
    virtual void pasteSubType( const QCString &subtype );
#endif
    virtual void clear();
    virtual void del();
    virtual void indent();
    virtual void setItalic( bool b );
    virtual void setBold( bool b );
    virtual void setUnderline( bool b );
    virtual void setFamily( const QString &f );
    virtual void setPointSize( int s );
    virtual void setColor( const QColor &c );
    virtual void setFont( const QFont &f );
    virtual void setVerticalAlignment( VerticalAlignment a );
    virtual void setAlignment( int a );
    virtual void setParagType( QStyleSheetItem::DisplayMode dm, QStyleSheetItem::ListStyle listStyle );
    virtual void setCursorPosition( int parag, int index );
    virtual void setSelection( int parag_from, int index_from, int parag_to, int index_to, int selNum = 0 );
    virtual void setSelectionAttributes( int selNum, const QColor &back, bool invertText );
    virtual void setModified( bool m );
    virtual void resetFormat();
    virtual void setUndoDepth( int d );
    virtual void setFormat( QTextFormat *f, int flags );
    virtual void ensureCursorVisible();
    virtual void placeCursor( const QPoint &pos, QTextCursor *c = 0 ) { placeCursor( pos, c, FALSE ); }
    virtual void moveCursor( CursorAction action, bool select );
    virtual void doKeyboardAction( KeyboardAction action );
    virtual void removeSelectedText( int selNum = 0 );
    virtual void removeSelection( int selNum = 0 );
    virtual void setCurrentFont( const QFont &f );
    virtual void setOverwriteMode( bool b ) { overWrite = b; }

    virtual void scrollToBottom();

    //    virtual void insert( const QString &text, bool indent = FALSE, bool checkNewLine = TRUE, bool removeSelected = TRUE );
    virtual void insert( const QString &text, bool indent, bool checkNewLine, bool removeSelected = TRUE );
    virtual void insertAt( const QString &text, int para, int index );
    virtual void removeParagraph( int para );
    virtual void insertParagraph( const QString &text, int para );

    virtual void setParagraphBackgroundColor( int para, const QColor &bg );
    virtual void clearParagraphBackground( int para );

    virtual void setUndoRedoEnabled( bool b );

signals:
    void textChanged();
    void selectionChanged();
    void copyAvailable( bool );
    void undoAvailable( bool yes );
    void redoAvailable( bool yes );
    void currentFontChanged( const QFont &f );
    void currentColorChanged( const QColor &c );
    void currentAlignmentChanged( int a );
    void currentVerticalAlignmentChanged( VerticalAlignment a );
    void cursorPositionChanged( QTextCursor *c );
    void cursorPositionChanged( int para, int pos );
    void returnPressed();
    void modificationChanged( bool m );

protected:
    void repaintChanged();
    void updateStyles();
    void drawContents( QPainter *p, int cx, int cy, int cw, int ch );
    bool event( QEvent *e );
    void keyPressEvent( QKeyEvent *e );
    void resizeEvent( QResizeEvent *e );
    void viewportResizeEvent( QResizeEvent* );
    void contentsMousePressEvent( QMouseEvent *e );
    void contentsMouseMoveEvent( QMouseEvent *e );
    void contentsMouseReleaseEvent( QMouseEvent *e );
    void contentsMouseDoubleClickEvent( QMouseEvent *e );
#ifndef QT_NO_WHEELEVENT
    void contentsWheelEvent( QWheelEvent *e );
#endif
#ifndef QT_NO_DRAGANDDROP
    void contentsDragEnterEvent( QDragEnterEvent *e );
    void contentsDragMoveEvent( QDragMoveEvent *e );
    void contentsDragLeaveEvent( QDragLeaveEvent *e );
    void contentsDropEvent( QDropEvent *e );
#endif
    bool focusNextPrevChild( bool next );
    QTextDocument *document() const;
    QTextCursor *textCursor() const;
    void setDocument( QTextDocument *doc );
    virtual QPopupMenu *createPopupMenu( const QPoint& pos );
    virtual QPopupMenu *createPopupMenu();
    void drawCursor( bool visible );

    void windowActivationChange( bool );

protected slots:
    virtual void doChangeInterval();

private slots:
    void formatMore();
    void doResize();
    void doAutoScroll();
    void blinkCursor();
    void setModified();
    void startDrag();
    void documentWidthChanged( int w );

private:
    struct UndoRedoInfo {
        enum Type { Invalid, Insert, Delete, Backspace, Return, RemoveSelected, Format, Alignment, ParagType };

        UndoRedoInfo( QTextDocument *dc );
        ~UndoRedoInfo();
        void clear();
        bool valid() const;

        QUndoRedoInfoPrivate *d;
        int id;
        int index;
        int eid;
        int eindex;
        QTextFormat *format;
        int flags;
        Type type;
        QTextDocument *doc;
        QMemArray<int> oldAligns;
        int newAlign;
        bool list;
        QStyleSheetItem::ListStyle listStyle;
        QValueList< QPtrVector<QStyleSheetItem> > oldStyles;
        QValueList<QStyleSheetItem::ListStyle> oldListStyles;
    };

private:
    void drawContents( QPainter * );
    virtual bool linksEnabled() const { return FALSE; }
    void init();
    void checkUndoRedoInfo( UndoRedoInfo::Type t );
    void updateCurrentFormat();
    bool handleReadOnlyKeyEvent( QKeyEvent *e );
    void makeParagVisible( QTextParag *p );
#ifndef QT_NO_MIME
    QCString pickSpecial(QMimeSource* ms, bool always_ask, const QPoint&);
#endif
#ifndef QT_NO_MIMECLIPBOARD
    void pasteSpecial(const QPoint&);
#endif
    void setFontInternal( const QFont &f );

    virtual void emitHighlighted( const QString & ) {}
    virtual void emitLinkClicked( const QString & ) {}

    void readFormats( QTextCursor &c1, QTextCursor &c2, int oldLen, QTextString &text, bool fillStyles = FALSE );
    void clearUndoRedo();
    void paintDocument( bool drawAll, QPainter *p, int cx = -1, int cy = -1, int cw = -1, int ch = -1 );
    void moveCursor( CursorAction action );
    void ensureFormatted( QTextParag *p );
    void placeCursor( const QPoint &pos, QTextCursor *c, bool link );

private:
    QTextDocument *doc;
    QTextCursor *cursor;
    QTimer *formatTimer, *scrollTimer, *changeIntervalTimer, *blinkTimer, *dragStartTimer;
    QTextParag *lastFormatted;
    int interval;
    UndoRedoInfo undoRedoInfo;
    QTextFormat *currentFormat;
    int currentAlignment;
    QPoint oldMousePos, mousePos;
    QPoint dragStartPos;
    QString onLink;
    WordWrap wrapMode;
    WrapPolicy wPolicy;
    int wrapWidth;
    QString pressedLink;
    QTextEditPrivate *d;
bool inDoubleClick : 1;
bool mousePressed : 1;
bool cursorVisible : 1;
bool blinkCursorVisible : 1;
bool readOnly : 1;
bool modified : 1;
bool mightStartDrag : 1;
bool inDnD : 1;
bool readonly : 1;
bool undoEnabled : 1;
bool overWrite : 1;
};

inline QTextDocument *QTextEdit::document() const
{
    return doc;
}

inline QTextCursor *QTextEdit::textCursor() const
{
    return cursor;
}

inline void QTextEdit::setCurrentFont( const QFont &f )
{
    QTextEdit::setFontInternal( f );
}

#endif
#endif //QT_NO_TEXTEDIT
#endif //QTEXTVIEW_H
