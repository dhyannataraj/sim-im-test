/***************************************************************************
 *                         qie.h  -  description
 *                         -------------------
 *                         begin                : Sun Mar 24 2002
 *                         copyright            : (C) 2002 by Vladimir Shutoff
 *                         email                : vovan@shutoff.ru
 ****************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************
 
 Based on wxIE Copyright (C) 2003 Lindsay Mathieson

*/

#ifndef QIE_X
#define QIE_X

#include "qactivex.h"

#include <exdisp.h>

class UI_EXPORT QIE : public QActiveX
{
	Q_OBJECT
public:
	QIE(QWidget *parent, const char *name=NULL);
	void refresh();
	void setReadOnly(bool);
	bool isReadOnly();
	QString text(bool asHTML=true);
	QString selectedText();
	void setText(const QString &text, const QString &context);
    QColor background();
    QColor foreground();
    void setForeground(const QColor&);
    void setBackground(const QColor&);
	void copy();
	void cut();
	void paste();
	void clear();
	void undo();
	void redo();
	void selectAll();
	void insert(const QString &text, bool bHtml, bool, bool);
	void append(const QString &text);
	TextFormat textFormat() const { return m_textFormat; }
	void setTextFormat(TextFormat);
	void setBold(bool);
	void setItalic(bool);
	void setUnderline(bool);
	void setCurrentFont(const QFont&);
	void setColor(const QColor&);
	bool hasSelectedText();
	bool isUndoAvailable();
	bool isRedoAvailable();
signals:
	void textChanged();
protected:
	virtual bool navigate(const QString&);
	AutoOleInterface<IWebBrowser2>		m_webBrowser;
	virtual void dispatch(const QString &name, vector<QVariant> &params, const vector<QString> &names);
	bool	m_bInProgress;
	QString m_text;
	bool	m_bEditChange;
	bool	m_bReadOnly;
	QColor	m_bg;
	bool	m_bgChanged;
	QColor	m_fg;
	bool	m_fgChanged;
	TextFormat m_textFormat;
	friend class HTMLDocument;
};

#endif
