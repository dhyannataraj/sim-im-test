/***************************************************************************
 *                         qactivex.h  -  description
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

#ifndef QACTIVE_X
#define QACTIVE_X

#include "simapi.h"
#include "stl.h"

#include <windows.h>
#include <docobj.h>
#include <mshtmhst.h>

#pragma warning(disable: 4251)  

namespace NS_ActiveX
{
	struct less_StringI
	{
		bool operator()(const QString& x, const QString& y) const
		{
			return x.lower() < y.lower();
		};
	};
};

template <class I> class AutoOleInterface
{
protected:
    I *m_interface;
public:
    explicit AutoOleInterface(I *pInterface = NULL) : m_interface(pInterface) {}

    AutoOleInterface(REFIID riid, IUnknown *pUnk) : m_interface(NULL)
	{
		QueryInterface(riid, pUnk);
	};

    AutoOleInterface(REFIID riid, IDispatch *pDispatch) : m_interface(NULL)
	{
		QueryInterface(riid, pDispatch);
	};

	AutoOleInterface(REFCLSID clsid, REFIID riid) : m_interface(NULL)
	{
		CreateInstance(clsid, riid);
	};

    AutoOleInterface(const AutoOleInterface<I>& ti) : m_interface(NULL)
    {
		operator = (ti);
    }

    AutoOleInterface<I>& operator = (const AutoOleInterface<I>& ti)
    {
		if (ti.m_interface)
			ti.m_interface->AddRef();
    	Free();
        m_interface = ti.m_interface;
        return *this;
    }

    AutoOleInterface<I>& operator = (I *&ti)
    {
    	Free();
        m_interface = ti;
        return *this;
    }

    ~AutoOleInterface()
    {
    	Free();
    };

    inline void Free()
    {
    	if (m_interface)
        	m_interface->Release();
        m_interface = NULL;
    };

    HRESULT QueryInterface(REFIID riid, IUnknown *pUnk)
	{
		Free();
		ASSERT(pUnk != NULL);
	    return pUnk->QueryInterface(riid, (void **) &m_interface);
	};

    HRESULT CreateInstance(REFCLSID clsid, REFIID riid)
    {
		Free();
	    return CoCreateInstance(clsid, NULL, CLSCTX_ALL, riid, (void **) &m_interface);
    };

    inline operator I *() const {return m_interface;}
    inline I* operator ->() {return m_interface;}
	inline I** GetRef()	{return &m_interface;}
	inline bool Ok() const	{return m_interface != NULL;}
};

class UI_EXPORT QActiveX : public QWidget
{
	Q_OBJECT
public:
	QActiveX(QWidget *parent, const char *name, REFCLSID clsid);
	~QActiveX();

	class ParamX 
	{
	public:
		USHORT	    flags;
        bool isPtr, isSafeArray;
		VARTYPE	    vt;
        QString		name;

        ParamX() : vt(VT_EMPTY) {}
		inline bool IsIn() const		{return (flags & IDLFLAG_FIN) != 0;}
		inline bool IsOut() const		{return (flags & IDLFLAG_FOUT) != 0;}
		inline bool IsRetVal() const	{return (flags & IDLFLAG_FRETVAL) != 0;}
	};
	typedef vector<ParamX>	ParamXArray;

    class FuncX 
    {
    public:
        QString		name;
        MEMBERID    memid;
		bool		hasOut;

        ParamX      retType;
		ParamXArray	params;
    };

	class PropX
	{
	public:
		QString	name;
        MEMBERID    memid;
        ParamX      type;
        ParamX      arg;
		bool		putByRef;

		PropX() : putByRef (false) {}
		inline bool CanGet() const {return type.vt != VT_EMPTY;}
		inline bool CanSet() const {return arg.vt != VT_EMPTY;}
	};

protected:
	void resizeEvent(QResizeEvent*);
	void paintEvent(QPaintEvent*);
	void mousePressEvent(QMouseEvent*);
	void mouseReleaseEvent(QMouseEvent*); 
	void mouseDoubleClickEvent(QMouseEvent*);
	void mouseMoveEvent(QMouseEvent*);
	void mouseEvent(UINT, QMouseEvent*);
	void focusInEvent(QFocusEvent*); 
	void focusOutEvent(QFocusEvent*); 
	
	virtual void dispatch(const QString &name, vector<QVariant> &params, const vector<QString> &names);
	virtual QPopupMenu *createPopupMenu(const QPoint& pos);

	void createActiveX(REFCLSID clsid);
	void getTypeInfo();
	void getTypeInfo(ITypeInfo *ti, bool defInterface, bool defEventSink);
	HRESULT connectAdvise(REFIID riid, IUnknown *eventSink);
	HRESULT ambientPropertyChanged(DISPID dispid);

	AutoOleInterface<IDispatch>					m_Dispatch;
    AutoOleInterface<IOleClientSite>			m_clientSite;
    AutoOleInterface<IUnknown>					m_ActiveX;
	AutoOleInterface<IOleObject>				m_oleObject;
	AutoOleInterface<IOleInPlaceObject>			m_oleInPlaceObject;
    AutoOleInterface<IOleInPlaceActiveObject>	m_oleInPlaceActiveObject;
    AutoOleInterface<IOleDocumentView>			m_docView;
    AutoOleInterface<IViewObject>				m_viewObject;
	AutoOleInterface<IDocHostUIHandler>			m_docHostUIHandler;

	DWORD	m_docAdviseCookie;
	HWND	m_oleObjectHWND;
    bool	m_bAmbientUserMode;

    typedef vector<FuncX>                                   FuncXArray;
    typedef vector<PropX>                                   PropXArray;
	typedef map<MEMBERID, int>	                            MemberIdMap;
    typedef map<QString, int, NS_ActiveX::less_StringI>		NameMap;

    typedef AutoOleInterface<IConnectionPoint>	OleConnectionPoint;
	typedef pair<OleConnectionPoint, DWORD>		OleConnection;
	typedef vector<OleConnection>				OleConnectionArray;

    FuncXArray		m_events;
    MemberIdMap     m_eventMemberIds;

	PropXArray		m_props;
    NameMap         m_propNames;

    FuncXArray      m_methods;
    NameMap         m_methodNames;

	OleConnectionArray	m_connections;

    inline int getEventCount() const {return m_events.size();}

	friend class FrameSite;
	friend class ActiveXEvents;
};

UI_EXPORT QString CnvBSTR(const unsigned short *str);

class UI_EXPORT UString
{
public:
	UString(const QString&);
	~UString();
	operator unsigned short *() { return m_data; }
protected:
	unsigned short *m_data;
};

#endif
