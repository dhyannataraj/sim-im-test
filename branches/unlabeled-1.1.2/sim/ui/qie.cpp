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

#include "qie.h"

#include <qvariant.h>

#include <exdisp.h>

#pragma warning(disable: 4310)

bool MSWVariantToVariant(VARIANTARG& va, QVariant& vx);
bool VariantToMSWVariant(QVariant& vx, VARIANTARG& va);

QIE::QIE(QWidget *parent, const char *name)
: QActiveX(parent, name, CLSID_WebBrowser)
{
	m_bInProgress	= true;
	m_bEditChange	= false;
	m_bgChanged		= false;
	m_fgChanged		= false;
	m_textFormat	= RichText;
	HRESULT hret;
	hret = m_webBrowser.QueryInterface(IID_IWebBrowser2, m_ActiveX);
	ASSERT(SUCCEEDED(hret));
	m_webBrowser->put_MenuBar(VARIANT_FALSE);
	m_webBrowser->put_AddressBar(VARIANT_FALSE);
	m_webBrowser->put_StatusBar(VARIANT_FALSE);
	m_webBrowser->put_ToolBar(VARIANT_FALSE);
	m_webBrowser->put_RegisterAsBrowser(VARIANT_TRUE);
	m_webBrowser->put_RegisterAsDropTarget(VARIANT_TRUE);
    VARIANTARG flags;
	flags.vt = VT_I4;
	flags.iVal = 0x2 | 0x4 | 0x8;
    VARIANTARG target;
	QVariant v("_SELF");
	VariantToMSWVariant(v, target);
    VARIANTARG post;
    VARIANTARG headers;
    m_webBrowser->Navigate( L"about:blank", &flags, &target, &post, &headers);
}

bool QIE::navigate(const QString&)
{
	return false;
}

void QIE::dispatch(const QString &name, vector<QVariant> &param, const vector<QString>&)
{
	OutputDebugStringA(name.latin1());
	OutputDebugStringA("\n");
	if ((name == "NavigateComplete2") ||
		(name == "DocumentComplete")){
		if (m_bInProgress){
			m_bInProgress = false;
			if (m_bEditChange){
				m_bEditChange = false;
				setReadOnly(m_bReadOnly);
			}
			QString text = m_text;
			m_text = "";
			append(text);
			if (m_bgChanged){
				m_bgChanged = false;
				setBackground(m_bg);
			}
			if (m_fgChanged){
				m_fgChanged = false;
				setForeground(m_fg);
			}
		}
	}
	if (name == "BeforeNavigate2"){
		if (m_bInProgress)
			return;
		QString url = param[1].asString();
		if (url == "about:blank")
			return;
		param[6] = !navigate(url);
	}
}

void QIE::refresh()
{
    VARIANTARG levelArg;
    levelArg.vt   = VT_I2;
    levelArg.iVal = 0;
    m_webBrowser->Refresh2(&levelArg);
}

void QIE::setReadOnly(bool mode)
{
	if (m_bInProgress){
		m_bEditChange = true;
		m_bReadOnly   = mode;
		return;
	}
    m_bAmbientUserMode = mode;
    ambientPropertyChanged(DISPID_AMBIENT_USERMODE);
};

bool QIE::isReadOnly()
{
	if (m_bEditChange)
		return m_bReadOnly;
    return m_bAmbientUserMode;
};

class HTMLDocument : public AutoOleInterface<IHTMLDocument2>
{
public:
	HTMLDocument() {}
	bool init(QIE *ie);
	operator IHTMLDocument2*() { return *this; }
};

bool HTMLDocument::init(QIE *ie)
{
	if (!ie->m_webBrowser.Ok())
		return false;
	IDispatch *iDisp = NULL;
    HRESULT hr = ie->m_webBrowser->get_Document(&iDisp);
    if ((hr != S_OK) || (iDisp == NULL))
    	return false;
    AutoOleInterface<IHTMLDocument2> hd(IID_IHTMLDocument2, iDisp);
    iDisp->Release();
    if (!hd.Ok())
		return false;
	*((AutoOleInterface<IHTMLDocument2>*)this) = hd;
	return true;
}

void QIE::append(const QString &html)
{
	if (m_bInProgress){
		m_text += html;
		return;
	}
	HTMLDocument hd;
	if (!hd.init(this))
		return;

	SAFEARRAY *sfArray = SafeArrayCreateVector(VT_VARIANT, 0, 1);
	if (sfArray == NULL)
		return;

	VARIANT *param;
	HRESULT hr = SafeArrayAccessData(sfArray,(LPVOID*) & param);
	UString str(html);
	BSTR bstr = SysAllocString(str);
	param->vt      = VT_BSTR;
	param->bstrVal = bstr;
	hr = SafeArrayUnaccessData(sfArray);
	hr = hd->write(sfArray);
	SysFreeString(bstr);
	SafeArrayDestroy(sfArray);
}

QString QIE::text(bool asHTML)
{
	HTMLDocument hd;
	if (!hd.init(this))
		return "";
	IHTMLElement *_body = NULL;
	hd->get_body(&_body);
	if (! _body)
		return "";
	AutoOleInterface<IHTMLElement> body(_body);
    BSTR text = NULL;
    HRESULT hr = E_FAIL;
	if (asHTML)
		hr = body->get_innerHTML(&text);
	else
		hr = body->get_innerText(&text);
    if (hr != S_OK)
    	return "";
    QString s = CnvBSTR(text);
    SysFreeString(text);
    return s;	
};

QColor QIE::background()
{
	if (m_bgChanged)
		return m_bg;
	HTMLDocument hd;
	if (!hd.init(this))
		return Qt::white;
	VARIANTARG va;
	hd->get_bgColor(&va);
	QVariant v;
	if (!MSWVariantToVariant(va, v))
		return Qt::white;
	unsigned rgb = v.asInt();
	return QColor(rgb);
}

QColor QIE::foreground()
{
	if (m_fgChanged)
		return m_fg;
	HTMLDocument hd;
	if (!hd.init(this))
		return Qt::black;
	VARIANTARG va;
	hd->get_fgColor(&va);
	QVariant v;
	if (!MSWVariantToVariant(va, v))
		return Qt::white;
	unsigned rgb = v.asInt();
	return QColor(rgb);
}

void QIE::setBackground(const QColor &c)
{
	if (m_bInProgress){
		m_bg = c;
		m_bgChanged = true;
		return;
	}
	HTMLDocument hd;
	if (!hd.init(this))
		return;
	IHTMLElement *_body = NULL;
	hd->get_body(&_body);
	if (!_body)
		return;
	AutoOleInterface<IHTMLElement> body(_body);
	VARIANTARG va;
	va.vt    = VT_I4;
	va.uiVal = c.rgb() & 0xFFFFFF;
	_body->setAttribute(L"bgColor", va, 0);
}

void QIE::setForeground(const QColor &c)
{
	if (m_bInProgress){
		m_fg = c;
		m_fgChanged = true;
		return;
	}
	HTMLDocument hd;
	if (!hd.init(this))
		return;
	IHTMLElement *_body = NULL;
	hd->get_body(&_body);
	if (!_body)
		return;
	AutoOleInterface<IHTMLElement> body(_body);
	VARIANTARG va;
	va.vt    = VT_I4;
	va.uiVal = c.rgb() & 0xFFFFFF;
	_body->setAttribute(L"fgColor", va, 0);
}

void QIE::setTextFormat(TextFormat format)
{
	m_textFormat = format;
}

void QIE::copy()
{
}

void QIE::cut()
{
}

void QIE::paste()
{
}

void QIE::setBold(bool)
{
}

void QIE::setItalic(bool)
{
}

void QIE::setUnderline(bool)
{
}

void QIE::setCurrentFont(const QFont&)
{
}

void QIE::setColor(const QColor&)
{
}

bool QIE::hasSelectedText()
{
	return false;
}

void QIE::setText(const QString&, const QString&)
{
}

void QIE::clear()
{
	setText("<html></html>", "");
}

void QIE::selectAll()
{
}

void QIE::undo()
{
}

void QIE::redo()
{
}

bool QIE::isUndoAvailable()
{
	return false;
}

bool QIE::isRedoAvailable()
{
	return false;
}

void QIE::insert(const QString&, bool, bool, bool)
{
}

QString QIE::selectedText()
{
	return "";
}



