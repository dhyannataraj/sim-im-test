/****************************************************************************
** $Id: qstylesheet.cpp,v 1.4 2002-07-11 01:26:48 shutoff Exp $
**
** Implementation of the QStyleSheet class
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

#include "qstylesheet.h"

#if QT_VERSION < 300
#ifndef QT_NO_RICHTEXT

#include "qrichtext_p.h"
#include "qlayout.h"
#include "qpainter.h"
#include "qcleanuphandler.h"

#include <stdio.h>

namespace Qt3 {

class QStyleSheetItemData
{
public:
    QStyleSheetItem::DisplayMode disp;
    int fontitalic;
    int fontunderline;
    int fontweight;
    int fontsize;
    int fontsizelog;
    int fontsizestep;
    int lineSpacing;
    QString fontfamily;
    QStyleSheetItem *parentstyle;
    QString stylename;
    int ncolumns;
    QColor col;
    bool anchor;
    int align;
    QStyleSheetItem::VerticalAlignment valign;
    int margin[5];
    QStyleSheetItem::ListStyle list;
    QStyleSheetItem::WhiteSpaceMode whitespacemode;
    QString contxt;
    bool selfnest;
    QStyleSheet* sheet;
};

/*!
  \class QStyleSheetItem qstylesheet.h
  \ingroup text
  \brief The QStyleSheetItem class provides an encapsulation of a set of text styles.

  A style sheet item consists of a name and a set of attributes that
  specifiy its font, color, etc. When used in a \link QStyleSheet
  style sheet\endlink (see styleSheet()), items define the name() of a
  rich text tag and the display property changes associated with it.

  The \link QStyleSheetItem::DisplayMode display mode\endlink
  attribute indicates whether the item is a block, an inline element
  or a list element; see setDisplayMode(). The treatment of whitespace
  is controlled by the \link QStyleSheetItem::WhiteSpaceMode white
  space mode\endlink; see setWhiteSpaceMode(). An item's margins are
  set with setMargin(), and line spacing is set with setLineSpacing().
  In the case of list items, the list style is set with
  setListStyle(). An item may be a hypertext link anchor; see
  setAnchor(). Other attributes are set with setAlignment(),
  setVerticalAlignment(),  setFontFamily(), setFontSize(),
  setFontWeight(), setFontItalic(), setFontUnderline() and setColor().
*/

/*! \enum QStyleSheetItem::AdditionalStyleValues
    \internal
*/

/*! \enum QStyleSheetItem::WhiteSpaceMode

  This enum defines the ways in which QStyleSheet can treat whitespace.  There are three values at present:

  \value WhiteSpaceNormal  any sequence of whitespace (including
  line-breaks) is equivalent to a single space.

  \value WhiteSpacePre  whitespace must be output exactly as given
  in the input.

  \value WhiteSpaceNoWrap  multiple spaces are collapsed as with
  WhiteSpaceNormal, but no automatic line-breaks occur. To break lines manually,
  use the \c{<br>} tag.

*/

/*! \enum QStyleSheetItem::Margin

  \value MarginLeft  left margin
  \value MarginRight  right margin
  \value MarginTop  top margin
  \value MarginBottom  bottom margin
  \value MarginAll  all margins (left, right, top and bottom)
  \value MarginVertical  top and bottom margins
  \value MarginHorizontal  left and right margins
  \value MarginFirstLine  margin (indentation) of the first line of a paragarph (in addition to the MarginLeft of the paragraph)
*/

/*!
  Constructs a new style named \a name for the stylesheet \a parent.

  All properties in QStyleSheetItem are initially in the "do not change" state,
  except \link QStyleSheetItem::DisplayMode display mode\endlink, which defaults
  to \c DisplayInline.
*/
QStyleSheetItem::QStyleSheetItem( QStyleSheet* parent, const QString& name )
{
    d = new QStyleSheetItemData;
    d->stylename = name.lower();
    d->sheet = parent;
    init();
    if (parent)
        parent->insert( this );
}

/*!
  Copy constructor. Constructs a copy of \a other that is
  not bound to any style sheet.
 */
QStyleSheetItem::QStyleSheetItem( const QStyleSheetItem & other )
{
    d = new QStyleSheetItemData;
    *d = *other.d;
}


/*!
  Destroys the style.  Note that QStyleSheetItem objects become owned
  by QStyleSheet when they are created.
 */
QStyleSheetItem::~QStyleSheetItem()
{
    delete d;
}



/*!
  Returns the style sheet this item is in.
 */
QStyleSheet* QStyleSheetItem::styleSheet()
{
    return d->sheet;
}

/*!
    \overload
  Returns the style sheet this item is in.
 */
const QStyleSheet* QStyleSheetItem::styleSheet() const
{
    return d->sheet;
}

/*!
  \internal
  Internal initialization
 */
void QStyleSheetItem::init()
{
    d->disp = DisplayInline;

    d->fontitalic = Undefined;
    d->fontunderline = Undefined;
    d->fontweight = Undefined;
    d->fontsize = Undefined;
    d->fontsizelog = Undefined;
    d->fontsizestep = 0;
    d->ncolumns = Undefined;
    d->col = QColor(); // !isValid()
    d->anchor = FALSE;
    d->align = Undefined;
    d->valign = VAlignBaseline;
    d->margin[0] = Undefined;
    d->margin[1] = Undefined;
    d->margin[2] = Undefined;
    d->margin[3] = Undefined;
    d->margin[4] = Undefined;
    d->list = QStyleSheetItem::ListDisc;
    d->whitespacemode = QStyleSheetItem::WhiteSpaceNormal;
    d->selfnest = TRUE;
    d->lineSpacing = Undefined;
}

/*!
  Returns the name of the style item.
*/
QString QStyleSheetItem::name() const
{
    return d->stylename;
}

/*!
  Returns the \link QStyleSheetItem::DisplayMode display mode\endlink
  of the style.

  \sa setDisplayMode()
 */
QStyleSheetItem::DisplayMode QStyleSheetItem::displayMode() const
{
    return d->disp;
}

/*! \enum QStyleSheetItem::DisplayMode

  This enum type defines the way adjacent elements are displayed.  The possible values are:

  \value DisplayBlock  elements are displayed as a rectangular block
  (e.g. \c{<p>...</p>}).

  \value DisplayInline  elements are displayed in a horizontally flowing
     sequence (e.g. \c{<em>...</em>}).

  \value DisplayListItem  elements are displayed in a vertical sequence
    (e.g. \c{<li>...</li>}).

  \value DisplayNone  elements are not displayed at all.
*/

/*!
  Sets the display mode of the style to \a m.

  \sa displayMode()
 */
void QStyleSheetItem::setDisplayMode(DisplayMode m)
{
    d->disp=m;
}


/*!
  Returns the alignment of this style. Possible values are AlignAuto, AlignLeft,
  AlignRight, AlignCenter and AlignJustify.

  \sa setAlignment(), Qt::AlignmentFlags
 */
int QStyleSheetItem::alignment() const
{
    return d->align;
}

/*!
  Sets the alignment to \a f. This only makes sense for styles with a
  \link QStyleSheetItem::DisplayMode display mode\endlink of
  DisplayBlock. Possible values are AlignAuto, AlignLeft, AlignRight,
  AlignCenter and AlignJustify.

  \sa alignment(), displayMode(), Qt::AlignmentFlags
 */
void QStyleSheetItem::setAlignment( int f )
{
    d->align = f;
}


/*!
  Returns the vertical alignment of the style. Possible values are
  VAlignBaseline, VAlignSub and VAlignSuper.

  psa setVerticalAlignment()
 */
QStyleSheetItem::VerticalAlignment QStyleSheetItem::verticalAlignment() const
{
    return d->valign;
}

/*! \enum QStyleSheetItem::VerticalAlignment

  This enum type defines the way elements are aligned vertically. This
  is supported for text elements only. The possible values are:

  \value VAlignBaseline align the baseline of the element (or the
  bottom, if the element doesn't have a baseline) with the baseline of
  the parent

  \value VAlignSub subscript the element

  \value VAlignSuper superscript the element

*/


/*!
  Sets the vertical alignment to \a valign. Possible values are
  VAlignBaseline, VAlignSub and VAlignSuper.

  The vertical alignment property is not inherited.

  \sa verticalAlignment()
 */
void QStyleSheetItem::setVerticalAlignment( VerticalAlignment valign )
{
    d->valign = valign;
}


/*!
  Returns TRUE if the style sets an italic font; otherwise returns FALSE.

 \sa setFontItalic(), definesFontItalic()
 */
bool QStyleSheetItem::fontItalic() const
{
    return d->fontitalic > 0;
}

/*!
    If \a italic is TRUE sets italic for the style; otherwise sets
    upright.

  \sa fontItalic(), definesFontItalic()
 */
void QStyleSheetItem::setFontItalic(bool italic)
{
    d->fontitalic = italic?1:0;
}

/*!
  Returns whether the style defines a font shape.  A style
  does not define any shape until setFontItalic() is called.

  \sa setFontItalic(), fontItalic()
 */
bool QStyleSheetItem::definesFontItalic() const
{
    return d->fontitalic != Undefined;
}

/*!
  Returns TRUE if the style sets an underlined font; otherwise returns FALSE.

 \sa setFontUnderline(), definesFontUnderline()
 */
bool QStyleSheetItem::fontUnderline() const
{
    return d->fontunderline > 0;
}

/*!
    If \a underline is TRUE sets underline for the style; otherwise sets
    no underline.

  \sa fontUnderline(), definesFontUnderline()
 */
void QStyleSheetItem::setFontUnderline(bool underline)
{
    d->fontunderline = underline?1:0;
}

/*!
  Returns whether the style defines a setting for the underline
  property of the font.  A style does not define this until
  setFontUnderline() is called.

  \sa setFontUnderline(), fontUnderline() */
bool QStyleSheetItem::definesFontUnderline() const
{
    return d->fontunderline != Undefined;
}


/*!
  Returns the font weight setting of the style. This is either a
  valid QFont::Weight or the value QStyleSheetItem::Undefined.

 \sa setFontWeight(), QFont
 */
int QStyleSheetItem::fontWeight() const
{
    return d->fontweight;
}

/*!
  Sets the font weight setting of the style to \a w.  Valid values are
  those defined by QFont::Weight.

  \sa QFont, fontWeight()
 */
void QStyleSheetItem::setFontWeight(int w)
{
    d->fontweight = w;
}

/*!
  Returns the logical font size setting of the style. This is either a valid
  size between 1 and 7  or QStyleSheetItem::Undefined.

 \sa setLogicalFontSize(), setLogicalFontSizeStep(), QFont::pointSize(), QFont::setPointSize()
 */
int QStyleSheetItem::logicalFontSize() const
{
    return d->fontsizelog;
}


/*!
  Sets the logical font size setting of the style to \a s.
  Valid logical sizes are 1 to 7.

 \sa logicalFontSize(), QFont::pointSize(), QFont::setPointSize()
 */
void QStyleSheetItem::setLogicalFontSize(int s)
{
    d->fontsizelog = s;
}

/*!
  Returns the logical font size step of this style.

  The default is 0. Tags such as \c big define \c +1; \c small defines
  \c -1.

  \sa setLogicalFontSizeStep()
 */
int QStyleSheetItem::logicalFontSizeStep() const
{
    return d->fontsizestep;
}

/*!
  Sets the logical font size step of this style to \a s.

  \sa logicalFontSizeStep()
 */
void QStyleSheetItem::setLogicalFontSizeStep( int s )
{
    d->fontsizestep = s;
}



/*!
  Sets the font size setting of the style to \a s points.

 \sa fontSize(), QFont::pointSize(), QFont::setPointSize()
 */
void QStyleSheetItem::setFontSize(int s)
{
    d->fontsize = s;
}

/*!
  Returns the font size setting of the style. This is either a valid
  point size or QStyleSheetItem::Undefined.

 \sa setFontSize(), QFont::pointSize(), QFont::setPointSize()
 */
int QStyleSheetItem::fontSize() const
{
    return d->fontsize;
}


/*!
  Returns the font family setting of the style. This is either a valid
  font family or QString::null if no family has been set.

 \sa setFontFamily(), QFont::family(), QFont::setFamily()
 */
QString QStyleSheetItem::fontFamily() const
{
    return d->fontfamily;
}

/*!
  Sets the font family setting of the style to \a fam.

 \sa fontFamily(), QFont::family(), QFont::setFamily()
 */
void QStyleSheetItem::setFontFamily( const QString& fam)
{
    d->fontfamily = fam;
}


/*!\obsolete
  Returns the number of columns for this style.

  \sa setNumberOfColumns(), displayMode(), setDisplayMode()

 */
int QStyleSheetItem::numberOfColumns() const
{
    return d->ncolumns;
}


/*!\obsolete
  Sets the number of columns for this style.  Elements in the style
  are divided into columns.

  This makes sense only if the style uses a block display mode
  (see QStyleSheetItem::DisplayMode).

  \sa numberOfColumns()
 */
void QStyleSheetItem::setNumberOfColumns(int ncols)
{
    if (ncols > 0)
        d->ncolumns = ncols;
}


/*!
  Returns the text color of this style or an invalid color
  if no color has been set.

  \sa setColor() QColor::isValid()
 */
QColor QStyleSheetItem::color() const
{
    return d->col;
}

/*!
  Sets the text color of this style to \a c.

  \sa color()
 */
void QStyleSheetItem::setColor( const QColor &c)
{
    d->col = c;
}

/*!
  Returns whether this style is an anchor.

  \sa setAnchor()
 */
bool QStyleSheetItem::isAnchor() const
{
    return d->anchor;
}

/*!
    If \a anc is TRUE sets this style to be an anchor (hypertext link);
    otherwise sets it to not be an anchor. Elements in this style have
    connections to other documents or anchors.

  \sa isAnchor()
 */
void QStyleSheetItem::setAnchor(bool anc)
{
    d->anchor = anc;
}


/*!
  Returns the whitespace mode.

  \sa setWhiteSpaceMode() WhiteSpaceMode
 */
QStyleSheetItem::WhiteSpaceMode QStyleSheetItem::whiteSpaceMode() const
{
    return d->whitespacemode;
}

/*!
  Sets the whitespace mode to \a m.
  \sa WhiteSpaceMode
 */
void QStyleSheetItem::setWhiteSpaceMode(WhiteSpaceMode m)
{
    d->whitespacemode = m;
}


/*!
  Returns the width of margin \a m in pixels.

  The margin, \a m, can be \c MarginLeft, \c MarginRight,
  \c MarginTop, \c MarginBottom, \c MarginAll, \c MarginVertical or \c
  MarginHorizontal.

  \sa setMargin() Margin
 */
int QStyleSheetItem::margin(Margin m) const
{
    return d->margin[m];
}


/*!
  Sets the width of margin \a m to \a v pixels.

  The margin, \a m, can be \c MarginLeft, \c MarginRight,
  \c MarginTop, \c MarginBottom, \c MarginAll, \c MarginVertical or \c
  MarginHorizontal.  The value \a v must be >= 0.

  \sa margin()
 */
void QStyleSheetItem::setMargin(Margin m, int v)
{
    if (m == MarginAll ) {
        d->margin[0] = v;
        d->margin[1] = v;
        d->margin[2] = v;
        d->margin[3] = v;
        d->margin[4] = v;
    } else if (m == MarginVertical ) {
        d->margin[MarginTop] = v;
        d->margin[MarginBottom] = v;
    } else if (m == MarginHorizontal ) {
        d->margin[MarginLeft] = v;
        d->margin[MarginRight] = v;
    } else {
        d->margin[m] = v;
    }
}


/*!
  Returns the list style of the style.

  \sa setListStyle() ListStyle
 */
QStyleSheetItem::ListStyle QStyleSheetItem::listStyle() const
{
    return d->list;
}

/*! \enum QStyleSheetItem::ListStyle

  This enum type defines how the items in a list are prefixed when
  displayed.  The currently defined values are:

  \value ListDisc  a filled circle (i.e. a bullet)
  \value ListCircle  an unfilled circle
  \value ListSquare  a filled square
  \value ListDecimal  an integer in base 10: \e 1, \e 2, \e 3, ...
  \value ListLowerAlpha  a lowercase letter: \e a, \e b, \e c, ...
  \value ListUpperAlpha  an uppercase letter: \e A, \e B, \e C, ...
*/
/*!
  Sets the list style of the style to \a s.

  This is used by nested elements that have a display mode of
  \c DisplayListItem.

  \sa listStyle() DisplayMode ListStyle
 */
void QStyleSheetItem::setListStyle(ListStyle s)
{
    d->list=s;
}


/*!  Returns a space-separated list of names of styles that may
  contain elements of this style. If nothing has been set, contexts()
  returns an empty string, which indicates that this style can be
  nested everywhere.

  \sa setContexts()
 */
QString QStyleSheetItem::contexts() const
{
    return d->contxt;
}

/*!
  Sets a space-separated list of names of styles that may contain
  elements of this style. If \a c is empty, the style can be nested
  everywhere.

  \sa contexts()
 */
void QStyleSheetItem::setContexts( const QString& c)
{
    d->contxt = QChar(' ') + c + QChar(' ');
}

/*!
  Returns TRUE if this style can be nested into an element
  of style \a s; otherwise returns FALSE.

  \sa contexts(), setContexts()
 */
bool QStyleSheetItem::allowedInContext( const QStyleSheetItem* s) const
{
    if ( d->contxt.isEmpty() )
        return TRUE;
    return d->contxt.find( QChar(' ')+s->name()+QChar(' ')) != -1;
}


/*!
  Returns TRUE if this style has self-nesting enabled; otherwise
  returns FALSE.

  \sa setSelfNesting()
 */
bool QStyleSheetItem::selfNesting() const
{
    return d->selfnest;
}

/*!
  Sets the self-nesting property for this style to \a nesting.

  In order to support "dirty" HTML, paragraphs \c{<p>} and list items
  \c{<li>} are not self-nesting. This means that starting a new
  paragraph or list item automatically closes the previous one.

  \sa selfNesting()
 */
void QStyleSheetItem::setSelfNesting( bool nesting )
{
    d->selfnest = nesting;
}

/*! Sets the linespacing to be \a ls pixels */

void QStyleSheetItem::setLineSpacing( int ls )
{
    d->lineSpacing = ls;
}

/*! Returns the linespacing */

int QStyleSheetItem::lineSpacing() const
{
    return d->lineSpacing;
}

//************************************************************************




//************************************************************************


/*!
  \class QStyleSheet qstylesheet.h
  \ingroup text
  \brief The QStyleSheet class is a collection of styles for rich text
  rendering and a generator of tags.

  \ingroup graphics
  \ingroup helpsystem

  By creating QStyleSheetItem objects for a style sheet you build a
  definition of a set of tags.  This definition will be used by the
  internal rich text rendering system to parse and display text
  documents to which the style sheet applies. Rich text is normally
  visualized in a QTextView or a QTextBrowser. However, QLabel,
  QWhatsThis and QMessageBox also support it, and other classes are
  likely to follow. With QSimpleRichText it is possible to use the
  rich text renderer for custom widgets as well.

  The default QStyleSheet object has the following style bindings,
  sorted by structuring bindings, anchors, character style bindings
  (i.e. inline styles), special elements such as horizontal lines or
  images, and other tags. In addition, rich text supports simple HTML
  tables.

  The structuring tags are
  \list
    \i \c{<qt>}...\c{</qt>}
	- A Qt rich text document. It understands the following attributes:
	\list
	\i title
	- The caption of the document. This attribute is easily accessible with
	QTextView::documentTitle().
	\i type
	- The type of the document. The default type is \c page . It
	  indicates that the document is displayed in a page of its
	  own. Another style is \c detail, which can be used to
	  explain certain expressions in more detail in a few
	  sentences. The QTextBrowser will then keep the current page
	  and display the new document in a small popup similar to
	  QWhatsThis. Note that links will not work in documents with
	  \c{<qt type="detail">...</qt>}.
	\i bgcolor
	- The background color, for example \c bgcolor="yellow" or \c
	  bgcolor="#0000FF".
	\i background
	- The background pixmap, for example \c
	  background="granit.xpm". The pixmap name will be resolved by
	  a QMimeSourceFactory().
	\i text
	- The default text color, for example \c text="red".
	\i link
	- The link color, for example \c link="green".
	\endlist
    \i \c{<h1>...</h1>}
	- A top-level heading.
    \i \c{<h2>...</h2>}
	- A sublevel heading.
    \i \c{<h3>...</h3>}
	- A sub-sublevel heading.
    \i \c{<p>...</p>}
	- A left-aligned paragraph. Adjust the alignment with
	the  \c align attribute. Possible values are
	\c left, \c right and \c center.
    \i \c{<center>...</center>}
	- A centered paragraph.
    \i \c{<blockquote>...</blockquote>}
	- An indented paragraph that is useful for quotes.
    \i \c{<ul>...</ul>}
	- An unordered list. You can also pass a type argument to
	define the bullet style. The default is \c type=disc; other
	types are \c circle and \c square.
    \i \c{<ol>...</ol>}
	- An ordered list. You can also pass a type argument to define
	the enumeration label style. The default is \c type="1"; other
	types are \c "a" and \c "A".
    \i <tt>&lt;li&gt;</tt>...<tt>&lt;/li&gt;</tt>
	- A list item. This tag can be used only within the context of
	\c ol or \c ul.
    \i \c{<pre>...</pre>}
	- For larger chunks of code. Whitespaces in the contents are preserved.
	For small bits of code use the inline-style \c code.
   \endlist

   Anchors and links are done with a single tag:
   \list
    \i \c{<a>...</a>}
	- An anchor or link. The reference target is defined in the \c
	  href attribute of the tag as in \c{<a
	  href="target.qml">...</a>}. You can also specify an
	  additional anchor within the specified target document, for
	  example \c{<a href="target.qml#123">...</a>}. If \c a is
	  meant to be an anchor, the reference source is given in the
	  \c name attribute.
  \endlist

   The default character style bindings are
   \list
    \i \c{<em>...</em>}
	- Emphasized. By default this is the same as
	\c{<i>...</i>} (italic).
    \i \c{<strong>...</strong>}
	- Strong. By default this is the same as
	\c{<b>...</b>} (bold).
    \i \c{<i>...</i>}
	- Italic font style.
    \i  \c{<b>...</b>}
	- Bold font style.
    \i  \c{<u>...</u>}
	- Underlined font style.
    \i  \c{<big>...</big>}
	- A larger font size.
    \i  \c{<small>...</small>}
	- A smaller font size.
    \i  \c{<code>...</code>}
	- Indicates code. By default this is the same as
	\c{<tt>...</tt>} (typewriter). For
	larger junks of code use the block-tag \c pre.
    \i  \c{<tt>...</tt>}
	- Typewriter font style.
    \i  \c{<font>...</font>}
	- Customizes the font size, family  and text color. The tag understands
	the following  attributes:
	\list
	\i color
	- The text color, for example \c color="red" or \c color="#FF0000".
	\i size
	- The logical size of the font. Logical sizes 1 to 7 are supported.
	 The value may either be absolute (for example,
	\c size=3) or relative (\c size=-2). In the latter case the sizes
	are simply added.
	\i face
	- The family of the font, for example \c face=times.
	\endlist
   \endlist

   Special elements are:
   \list
    \i \c{<img>}
	- An image. The image name for the mime source
	factory is given in the source attribute, for example
	\c{<img src="qt.xpm">}
	The image tag also understands the attributes \c width and \c
	height that determine the size of the image. If the pixmap
	does not fit the specified size it will be scaled
	automatically (by using QImage::smoothScale()).

	The \c align attribute determines where the image is
	placed. By default, an image is placed inline just like a
	normal character. Specify \c left or \c right to place the
	image at the respective side.
    \i \c{<hr>}
	- A horizonal line.
    \i \c{<br>}
	- A line break.
  \endlist

  Another tag not in any of the above cathegories is
  \list
  \i \c{<nobr>...</nobr>}
	- No break. Prevents word wrap.
  \endlist

  In addition, rich text supports simple HTML tables. A table consists
  of one or more rows each of which contains one or more cells. Cells
  are either data cells or header cells, depending on their
  content. Cells which span rows and columns are supported.

 \list
   \i \c{<table>...</table>}
   - A table. Tables support the following attributes:
	\list
	\i bgcolor
	- The background color.
	\i width
	- The table width. This is either an absolute pixel width or a relative
	percentage of the table's width, for example \c width=80%.
	\i border
	- The width of the table border. The default is 0 (= no border).
	\i cellspacing
	- Additional space around the table cells. The default is 2.
	\i cellpadding
	- Additional space around the contents of table cells. The default is 1.
	\endlist
   \i \c{<tr>...</tr>}
   - A table row. This is only valid within a \c table. Rows support
     the following attribute:
	\list
	\i bgcolor
	- The background color.
	\endlist
   \i \c{<th>...</th>}
   - A table header cell. Similar to \c td, but defaults to center alignment
     and a bold font.
   \i \c{<td>...</td>}
   - A table data cell. This is only valid within a \c tr. Cells
     support the following attributes:
	\list
	\i bgcolor
	- The background color.
	\i width
	- The cell width. This is either an absolute pixel width or a relative
	percentage of table's width, for example \c width=50%.
	\i colspan
	- Specifies how many columns this cell spans. The default is 1.
	\i rowspan
	- Specifies how many rows this cell spans. The default is 1.
	\i align
	- Alignment; possible values are \c left, \c right, and \c center. The
	default is left.
	\endlist
   \endlist
*/

/*!
  Creates a style sheet with parent \a parent and name \a name.  Like
  any QObject it will be deleted when its parent is
  destroyed (if the child still exists).

  By default the style sheet has the tag definitions defined above.
*/
QStyleSheet::QStyleSheet( QObject *parent, const char *name )
        : QObject( parent, name )
{
    init();
}

/*!
  Destroys the style sheet.  All styles inserted into the style sheet
  will be deleted.
*/
QStyleSheet::~QStyleSheet()
{
}

/*!
  \internal
  Initialized the style sheet to the basic Qt style.
*/
void QStyleSheet::init()
{
    styles.setAutoDelete( TRUE );

    nullstyle  = new QStyleSheetItem( this,
                                      QString::fromLatin1("") );

    QStyleSheetItem*  style;

    style = new QStyleSheetItem( this, "qml" ); // compatibility
    style->setDisplayMode( QStyleSheetItem::DisplayBlock );

    style = new QStyleSheetItem( this, QString::fromLatin1("qt") );
    style->setDisplayMode( QStyleSheetItem::DisplayBlock );
    //style->setMargin( QStyleSheetItem::MarginAll, 4 );

    style = new QStyleSheetItem( this, QString::fromLatin1("a") );
    style->setAnchor( TRUE );

    style = new QStyleSheetItem( this, QString::fromLatin1("em") );
    style->setFontItalic( TRUE );

    style = new QStyleSheetItem( this, QString::fromLatin1("i") );
    style->setFontItalic( TRUE );

    style = new QStyleSheetItem( this, QString::fromLatin1("big") );
    style->setLogicalFontSizeStep( 1 );
    style = new QStyleSheetItem( this, QString::fromLatin1("large") ); // compatibility
    style->setLogicalFontSizeStep( 1 );

    style = new QStyleSheetItem( this, QString::fromLatin1("small") );
    style->setLogicalFontSizeStep( -1 );

    style = new QStyleSheetItem( this, QString::fromLatin1("strong") );
    style->setFontWeight( QFont::Bold);

    style = new QStyleSheetItem( this, QString::fromLatin1("b") );
    style->setFontWeight( QFont::Bold);

    style = new QStyleSheetItem( this, QString::fromLatin1("h1") );
    style->setFontWeight( QFont::Bold);
    style->setLogicalFontSize(6);
    style->setDisplayMode(QStyleSheetItem::DisplayBlock);
    style-> setMargin(QStyleSheetItem::MarginTop, 12);
    style-> setMargin(QStyleSheetItem::MarginBottom, 6);

    style = new QStyleSheetItem( this, QString::fromLatin1("h2") );
    style->setFontWeight( QFont::Bold);
    style->setLogicalFontSize(5);
    style->setDisplayMode(QStyleSheetItem::DisplayBlock);
    style-> setMargin(QStyleSheetItem::MarginTop, 10);
    style-> setMargin(QStyleSheetItem::MarginBottom, 5);

    style = new QStyleSheetItem( this, QString::fromLatin1("h3") );
    style->setFontWeight( QFont::Bold);
    style->setLogicalFontSize(4);
    style->setDisplayMode(QStyleSheetItem::DisplayBlock);
    style-> setMargin(QStyleSheetItem::MarginTop, 8);
    style-> setMargin(QStyleSheetItem::MarginBottom, 4);

    style = new QStyleSheetItem( this, QString::fromLatin1("h4") );
    style->setFontWeight( QFont::Bold);
    style->setLogicalFontSize(3);
    style->setDisplayMode(QStyleSheetItem::DisplayBlock);
    style-> setMargin(QStyleSheetItem::MarginTop, 8);
    style-> setMargin(QStyleSheetItem::MarginBottom, 4);

    style = new QStyleSheetItem( this, QString::fromLatin1("h5") );
    style->setFontWeight( QFont::Bold);
    style->setLogicalFontSize(2);
    style->setDisplayMode(QStyleSheetItem::DisplayBlock);
    style-> setMargin(QStyleSheetItem::MarginTop, 8);
    style-> setMargin(QStyleSheetItem::MarginBottom, 4);

    style = new QStyleSheetItem( this, QString::fromLatin1("p") );
    style->setDisplayMode(QStyleSheetItem::DisplayBlock);
    style-> setMargin(QStyleSheetItem::MarginVertical, 8);
    style->setSelfNesting( FALSE );

    style = new QStyleSheetItem( this, QString::fromLatin1("center") );
    style->setDisplayMode(QStyleSheetItem::DisplayBlock);
    style->setAlignment( AlignCenter );

    style = new QStyleSheetItem( this, QString::fromLatin1("twocolumn") );
    style->setDisplayMode(QStyleSheetItem::DisplayBlock);
    style->setNumberOfColumns( 2 );

    style =  new QStyleSheetItem( this, QString::fromLatin1("multicol") );
    style->setDisplayMode(QStyleSheetItem::DisplayBlock);
    (void) new QStyleSheetItem( this, QString::fromLatin1("font") );

    style = new QStyleSheetItem( this, QString::fromLatin1("ul") );
    style->setDisplayMode(QStyleSheetItem::DisplayBlock);
    style-> setMargin(QStyleSheetItem::MarginVertical, 4);

    style = new QStyleSheetItem( this, QString::fromLatin1("ol") );
    style->setDisplayMode(QStyleSheetItem::DisplayBlock);
    style->setListStyle( QStyleSheetItem::ListDecimal );
    style-> setMargin(QStyleSheetItem::MarginVertical, 4);

    style = new QStyleSheetItem( this, QString::fromLatin1("li") );
    style->setDisplayMode(QStyleSheetItem::DisplayListItem);
    style->setSelfNesting( FALSE );
    style->setContexts(QString::fromLatin1("ol ul"));
    style-> setMargin(QStyleSheetItem::MarginVertical, 4);

    style = new QStyleSheetItem( this, QString::fromLatin1("code") );
    style->setFontFamily( QString::fromLatin1("courier") );

    style = new QStyleSheetItem( this, QString::fromLatin1("tt") );
    style->setFontFamily( QString::fromLatin1("courier") );

    new QStyleSheetItem(this, QString::fromLatin1("img"));
    new QStyleSheetItem(this, QString::fromLatin1("br"));
    new QStyleSheetItem(this, QString::fromLatin1("hr"));
    style = new QStyleSheetItem(this, QString::fromLatin1("sub"));
    style->setVerticalAlignment( QStyleSheetItem::VAlignSub );
    style = new QStyleSheetItem(this, QString::fromLatin1("sup"));
    style->setVerticalAlignment( QStyleSheetItem::VAlignSuper );

    style = new QStyleSheetItem( this, QString::fromLatin1("pre") );
    style->setFontFamily( QString::fromLatin1("courier") );
    style->setDisplayMode(QStyleSheetItem::DisplayBlock);
    style->setWhiteSpaceMode(QStyleSheetItem::WhiteSpacePre);

    style = new QStyleSheetItem( this, QString::fromLatin1("blockquote") );
    style->setDisplayMode(QStyleSheetItem::DisplayBlock);
    style->setMargin(QStyleSheetItem::MarginHorizontal, 40 );

    style = new QStyleSheetItem( this, QString::fromLatin1("head") );
    style->setDisplayMode(QStyleSheetItem::DisplayNone);
    style = new QStyleSheetItem( this, QString::fromLatin1("div") );
    style->setDisplayMode(QStyleSheetItem::DisplayBlock) ;
    style = new QStyleSheetItem( this, QString::fromLatin1("dl") );
    style->setDisplayMode(QStyleSheetItem::DisplayBlock);
    style = new QStyleSheetItem( this, QString::fromLatin1("dt") );
    style->setDisplayMode(QStyleSheetItem::DisplayBlock);
    style->setContexts(QString::fromLatin1("dl") );
    style = new QStyleSheetItem( this, QString::fromLatin1("dd") );
    style->setDisplayMode(QStyleSheetItem::DisplayBlock);
    style->setMargin(QStyleSheetItem::MarginLeft, 30);
    style->setContexts(QString::fromLatin1("dt dl") );
    style = new QStyleSheetItem( this, QString::fromLatin1("u") );
    style->setFontUnderline( TRUE);
    style = new QStyleSheetItem( this, QString::fromLatin1("nobr") );
    style->setWhiteSpaceMode( QStyleSheetItem::WhiteSpaceNoWrap );
    style = new QStyleSheetItem( this, QString::fromLatin1("wsp") ); // qt extension for QTextEdit
    style->setWhiteSpaceMode( (QStyleSheetItem::WhiteSpaceMode) 3 ); // WhiteSpaceModeNoCompression

    // tables
    style = new QStyleSheetItem( this, QString::fromLatin1("table") );
    style = new QStyleSheetItem( this, QString::fromLatin1("tr") );
    style->setContexts(QString::fromLatin1("table"));
    style = new QStyleSheetItem( this, QString::fromLatin1("td") );
    style->setContexts(QString::fromLatin1("tr"));
    style = new QStyleSheetItem( this, QString::fromLatin1("th") );
    style->setFontWeight( QFont::Bold );
    style->setAlignment( Qt::AlignCenter );
    style->setContexts(QString::fromLatin1("tr"));

    style = new QStyleSheetItem( this, QString::fromLatin1("html") );
}



static QStyleSheet* defaultsheet = 0;
static QCleanupHandler<QStyleSheet> qt_cleanup_stylesheet;

/*!
  Returns the application-wide default style sheet. This style sheet is
  used by rich text rendering classes such as QSimpleRichText,
  QWhatsThis and QMessageBox to define the rendering style and
  available tags within rich text documents. It serves also as initial
  style sheet for the more complex render widgets QTextEdit and
  QTextBrowser.

  \sa setDefaultSheet()
*/
QStyleSheet* QStyleSheet::defaultSheet()
{
    if (!defaultsheet) {
        defaultsheet = new QStyleSheet();
        qt_cleanup_stylesheet.add( defaultsheet );
    }
    return defaultsheet;
}

/*!
  Sets the application-wide default style sheet to \a sheet, deleting
  any style sheet previously set. The ownership is transferred to
  QStyleSheet.

  \sa defaultSheet()
*/
void QStyleSheet::setDefaultSheet( QStyleSheet* sheet)
{
    if ( defaultsheet != sheet ) {
        if ( defaultsheet )
            qt_cleanup_stylesheet.remove( defaultsheet );
        delete defaultsheet;
    }
    defaultsheet = sheet;
    if ( defaultsheet )
        qt_cleanup_stylesheet.add( defaultsheet );
}

/*!\internal
  Inserts \a style.  Any tags generated after this time will be
  bound to this style.  Note that \a style becomes owned by the
  style sheet and will be deleted when the style sheet is destroyed.
*/
void QStyleSheet::insert( QStyleSheetItem* style )
{
    styles.insert(style->name(), style);
}


/*!
  Returns the style with name \a name or 0 if there is no such style.
 */
QStyleSheetItem* QStyleSheet::item( const QString& name)
{
    if ( name.isNull() )
        return 0;
    return styles[name];
}

/*!
    \overload
  Returns the style with name \a name or 0 if there is no such style (const version)
 */
const QStyleSheetItem* QStyleSheet::item( const QString& name) const
{
    if ( name.isNull() )
        return 0;
    return styles[name];
}


/*!
    \preliminary
  Generates an internal object for the tag called \a name, given the
  attributes \a attr, and using additional information provided
  by the mime source factory \a factory.

  \a context is the optional context of the document, i.e. the path to
  look for relative links. This becomes important if the text contains
  relative references, for example within image tags. QSimpleRichText
  always uses the default mime source factory (see
  \l{QMimeSourceFactory::defaultFactory()}) to resolve these references.
  The context will then be used to calculate the absolute path. See
  QMimeSourceFactory::makeAbsolute() for details.

  \a emptyTag and \a doc are for internal use only.

  This function should not (yet) be used in application code.
*/
QTextCustomItem* QStyleSheet::tag(  const QString& name,
                                    const QMap<QString, QString> &attr,
                                    const QString& context,
                                    const QMimeSourceFactory& factory,
                                    bool /*emptyTag */, QTextDocument *doc ) const
{
    static QString s_img = QString::fromLatin1("img");
    static QString s_hr = QString::fromLatin1("hr");

    const QStyleSheetItem* style = item( name );
    // first some known  tags
    if ( !style )
        return 0;
    if ( style->name() == s_img )
        return new QTextImage( doc, attr, context, (QMimeSourceFactory&)factory );
    if ( style->name() == s_hr )
        return new QTextHorizontalLine( doc, attr, context, (QMimeSourceFactory&)factory  );
    return 0;
}


/*!
  Auxiliary function. Converts the plain text string \a plain to a
  rich text formatted paragraph while preserving its look.

  \a mode defines the whitespace mode. Possible values are \c
  QStyleSheetItem::WhiteSpacePre (no wrapping, all whitespaces
  preserved) and \c QStyleSheetItem::WhiteSpaceNormal (wrapping,
  simplified whitespaces).

  \sa escape()
 */
QString QStyleSheet::convertFromPlainText( const QString& plain, QStyleSheetItem::WhiteSpaceMode mode )
{
    int col = 0;
    QString rich;
    rich += "<p>";
    for ( int i = 0; i < int(plain.length()); ++i ) {
        if ( plain[i] == '\n' ){
            if ( col == 1 )
                rich += "<p></p>";
            else
                rich += "<br>";
            col = 0;
        }
        else if ( mode == QStyleSheetItem::WhiteSpacePre && plain[i] == '\t' ){
            rich += 0x00a0U;
            while ( col % 4 ) {
                rich += 0x00a0U;
                ++col;
            }
        }
        else if ( mode == QStyleSheetItem::WhiteSpacePre && plain[i].isSpace() )
            rich += 0x00a0U;
        else if ( plain[i] == '<' )
            rich +="&lt;";
        else if ( plain[i] == '>' )
            rich +="&gt;";
        else if ( plain[i] == '&' )
            rich +="&amp;";
        else
            rich += plain[i];
        ++col;
    }
    rich += "</p>";
    return rich;
}

/*!
  Auxiliary function. Converts the plain text string \a plain to a
  rich text formatted string with any HTML meta-characters escaped.

  \sa convertFromPlainText()
 */
QString QStyleSheet::escape( const QString& plain)
{
    QString rich;
    for ( int i = 0; i < int(plain.length()); ++i ) {
        if ( plain[i] == '<' )
            rich +="&lt;";
        else if ( plain[i] == '>' )
            rich +="&gt;";
        else if ( plain[i] == '&' )
            rich +="&amp;";
        else
            rich += plain[i];
    }
    return rich;
}

// Must doc this enum somewhere, and it is logically related to QStyleSheet

/*!
  \enum Qt::TextFormat

  This enum is used in widgets that can display both plain text and
  rich text, e.g. QLabel. It is used for deciding whether a text
  string should be interpreted as one or the other. This is
  normally done by passing one of the enum values to a setTextFormat()
  function.

  \value PlainText  The text string is interpreted as a plain text string.

  \value RichText The text string is interpreted as a rich text string
  using the current QStyleSheet::defaultSheet().

  \value AutoText The text string is interpreted as for \c RichText if
  QStyleSheet::mightBeRichText() returns TRUE, otherwise as for \c
  PlainText.
*/

/*!
  Returns TRUE if the string \a text is likely to be rich text;
  otherwise returns FALSE.

  Note: The function uses a fast and therefore simple heuristic. It
  mainly checks whether there is something that looks like a tag
  before the first line break. Although the result may be correct for
  most common cases, there is no guarantee.
*/
bool QStyleSheet::mightBeRichText( const QString& text)
{
    if ( text.isEmpty() )
        return FALSE;
    if ( text.left(5).lower() == "<!doc" )
        return TRUE;
    int open = 0;
    while ( open < int(text.length()) && text[open] != '<'
            && text[open] != '\n' && text[open] != '&')
        ++open;
    if ( text[open] == '&' ) {
        if ( text.mid(open+1,3) == "lt;" )
            return TRUE; // support desperate attempt of user to see <...>
    } else if ( text[open] == '<' ) {
        int close = text.find('>', open);
        if ( close > -1 ) {
            QString tag;
            for (int i = open+1; i < close; ++i) {
                if ( text[i].isDigit() || text[i].isLetter() )
                    tag += text[i];
                else if ( !tag.isEmpty() && text[i].isSpace() )
                    break;
                else if ( !text[i].isSpace() && (!tag.isEmpty() || text[i] != '!' ) )
                    return FALSE; // that's not a tag
            }
            return defaultSheet()->item( tag.lower() ) != 0;
        }
    }
    return FALSE;
}


/*! \fn void QStyleSheet::error( const QString& msg) const

  This virtual function is called when an error occurs when
  processing rich text. Reimplement it if you need to catch
  error messages.

  Errors might occur if some rich text strings contain tags that are
  not understood by the stylesheet, if some tags are nested incorrectly, or
  if tags are not closed properly.

  \a msg is the error message.
*/
void QStyleSheet::error( const QString& ) const
{
}


/*!
  Scales the font \a font to the appropriate physical point size
  corresponding to the logical font size \a logicalSize.

  When calling this function, \a font has a point size corresponding to
  the logical font size 3.

  Logical font sizes range from 1 to 7, with 1 being the smallest.

  \sa QStyleSheetItem::logicalFontSize(),
  QStyleSheetItem::logicalFontSizeStep(), QFont::setPointSize()
 */
void QStyleSheet::scaleFont( QFont& font, int logicalSize ) const
{
    if ( logicalSize < 1 )
        logicalSize = 1;
    if ( logicalSize > 7 )
        logicalSize = 7;
    int basePointSize = font.pointSize();
    int s;
    switch ( logicalSize ) {
    case 1:
        s =  basePointSize/2;
        break;
    case 2:
        s = (8 * basePointSize) / 10;
        break;
    case 4:
        s =  (12 * basePointSize) / 10;
        break;
    case 5:
        s = (15 * basePointSize) / 10;
        break;
    case 6:
        s = 2 * basePointSize;
        break;
    case 7:
        s = (24 * basePointSize) / 10;
        break;
    default:
        s = basePointSize;
    }
    font.setPointSize( s );
}

}

#ifndef _WINDOWS
#include "qstylesheet.moc"
#endif

#endif
#endif // QT_NO_RICHTEXT
