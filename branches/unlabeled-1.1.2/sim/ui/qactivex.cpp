/***************************************************************************
 *                         qactivex.cpp  -  description
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

#include "qactivex.h"

#include <qpainter.h>
#include <qvariant.h>
#include <qdatetime.h>

#include <oleidl.h>
#include <winerror.h>
#include <idispids.h>
#include <olectl.h>

#ifdef _DEBUG
#define DEBUG_LOG(A)	OutputDebugStringA(A "\n")
#else
#define DEBUG_LOG(A)
#endif

class QOleInit
{
	public:
    static IMalloc *GetIMalloc();

    QOleInit();
    ~QOleInit();
};

#define DECLARE_OLE_UNKNOWN(cls)\
	private:\
    class TAutoInitInt\
    {\
    	public:\
        LONG l;\
        TAutoInitInt() : l(0) {}\
    };\
    TAutoInitInt refCount, lockCount;\
    QOleInit oleInit;\
	static void _GetInterface(cls *self, REFIID iid, void **_interface, const char *&desc);\
    public:\
    LONG GetRefCount();\
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void ** ppvObject);\
	ULONG STDMETHODCALLTYPE AddRef();\
	ULONG STDMETHODCALLTYPE Release();\
    ULONG STDMETHODCALLTYPE AddLock();\
	ULONG STDMETHODCALLTYPE ReleaseLock()

#define DEFINE_OLE_TABLE(cls)\
	LONG cls::GetRefCount() {return refCount.l;}\
    HRESULT STDMETHODCALLTYPE cls::QueryInterface(REFIID iid, void ** ppvObject)\
    {\
        if (! ppvObject)\
        {\
            return E_FAIL;\
        };\
        const char *desc = NULL;\
        cls::_GetInterface(this, iid, ppvObject, desc);\
        if (! *ppvObject)\
        {\
            return E_NOINTERFACE;\
        };\
        ((IUnknown * )(*ppvObject))->AddRef();\
        return S_OK;\
    };\
    ULONG STDMETHODCALLTYPE cls::AddRef()\
    {\
        InterlockedIncrement(&refCount.l);\
        return refCount.l;\
    };\
    ULONG STDMETHODCALLTYPE cls::Release()\
    {\
    	if (refCount.l > 0)\
        {\
	    	InterlockedDecrement(&refCount.l);\
    	    if (refCount.l == 0)\
        	{\
            	delete this;\
	            return 0;\
	        };\
	        return refCount.l;\
        }\
        else\
        	return 0;\
    }\
    ULONG STDMETHODCALLTYPE cls::AddLock()\
    {\
        InterlockedIncrement(&lockCount.l);\
        return lockCount.l;\
    };\
    ULONG STDMETHODCALLTYPE cls::ReleaseLock()\
    {\
    	if (lockCount.l > 0)\
        {\
	        InterlockedDecrement(&lockCount.l);\
    	    return lockCount.l;\
        }\
        else\
        	return 0;\
    }\
    DEFINE_OLE_BASE(cls)

#define DEFINE_OLE_BASE(cls)\
	void cls::_GetInterface(cls *self, REFIID iid, void **_interface, const char *&desc)\
	{\
		*_interface = NULL;\
	    desc = NULL;

#define OLE_INTERFACE(_iid, _type)\
    if (IsEqualIID(iid, _iid))\
    {\
    	*_interface = (IUnknown *) (_type *) self;\
    	desc = # _iid;\
        return;\
    }

#define OLE_IINTERFACE(_face) OLE_INTERFACE(IID_##_face, _face)

#define OLE_INTERFACE_CUSTOM(func)\
    if (func(self, iid, _interface, desc))\
	{\
        return;\
	}

#define END_OLE_TABLE\
	}

QString CnvBSTR(const unsigned short *str)
{
	QString res;
	if (str == NULL)
		return res;
	for (; *str; str++)
		res += QChar(*str);
	return res;
}

bool MSWVariantToVariant(VARIANTARG& va, QVariant& vx)
{
	bool byRef = false;
	VARTYPE vt = va.vt;
	if (vt & VT_ARRAY)
		return false;
	if (vt & VT_BYREF)
	{
		byRef = true;
		vt &= ~(VT_BYREF);
	};
    switch(vt)
    {
	case VT_VARIANT:
		if (byRef)
			return MSWVariantToVariant(*va.pvarVal, vx);
		return false;
	case VT_I1:
	case VT_UI1:
		if (byRef)
			vx = (char) *va.pbVal;
		else
			vx = (char) va.bVal;
		return true;
    case VT_I2:
	case VT_UI2:
		if (byRef)
			vx = (long) *va.puiVal;
		else
			vx = (long) va.uiVal;
		return true;
    case VT_I4:
	case VT_UI4:
    case VT_INT:
	case VT_UINT:
	case VT_ERROR:
		if (byRef)
	        vx = (long) *va.pulVal;
		else
	        vx = (long) va.ulVal;
        return true;
	case VT_R4:
		if (byRef)
			vx = *va.pfltVal;
		else
			vx = va.fltVal;
		return true;
	case VT_R8:
		if (byRef)
			vx = *va.pdblVal;
		else
			vx = va.dblVal;
		return true;
	case VT_BOOL:
		if (byRef)
			vx = (*va.pboolVal ? true : false);
		else
			vx = (va.boolVal ? true : false);
		return true;
	case VT_CY:
		return false;
	case VT_DECIMAL:
		{
			double d = 0;
			HRESULT hr;
			if (byRef)
				hr = VarR8FromDec(va.pdecVal, &d);
			else
				hr = VarR8FromDec(&va.decVal, &d);

			vx = d;
			return SUCCEEDED(hr);
		};
	case VT_DATE:
		return false;
    case VT_BSTR:
		if (byRef)
	        vx = CnvBSTR(*va.pbstrVal);
		else
	        vx = CnvBSTR(va.bstrVal);
        return true;
	case VT_UNKNOWN:
		return false;
	case VT_DISPATCH:
		return false;
    default:
        return false;
    };
};

UString::UString(const QString &str)
{
	unsigned size = str.length();
	m_data = new unsigned short[size + 1];
	memcpy(m_data, str.unicode(), size * sizeof(unsigned short));
	m_data[size] = 0;
}

UString::~UString()
{
	delete[] m_data;
}

bool VariantToMSWVariant(QVariant& vx, VARIANTARG& va)
{
	bool byRef = false;
	VARTYPE vt = va.vt;

	if (vt & VT_ARRAY)
		return false;

	if (vt & VT_BYREF)
	{
		byRef = true;
		vt &= ~(VT_BYREF);
	};

    switch(vt)
    {
	case VT_VARIANT:
		if (byRef)
			return VariantToMSWVariant(vx, *va.pvarVal);
		return false;
	case VT_I1:
	case VT_UI1:
		if (byRef)
			*va.pbVal = (char)vx.asInt();
		else
			va.bVal = (char)vx.asInt();
		return true;
    case VT_I2:
	case VT_UI2:
		if (byRef)
			*va.puiVal = (short)vx.asInt();
		else
			va.uiVal = (short)vx.asInt();
		return true;
    case VT_I4:
	case VT_UI4:
    case VT_INT:
	case VT_UINT:
	case VT_ERROR:
		if (byRef)
	        *va.pulVal = vx.asInt();
		else
	        va.ulVal = vx.asInt();
        return true;
	case VT_R4:
		if (byRef)
			*va.pfltVal = (float)vx.asDouble();
		else
			va.fltVal = (float)vx.asDouble();
		return true;
	case VT_R8:
		if (byRef)
			*va.pdblVal = vx.asDouble();
		else
			va.dblVal = vx.asDouble();
		return true;
	case VT_BOOL:
		if (byRef)
			*va.pboolVal = vx.asBool();
		else
			va.boolVal = vx.asBool();
		return true;
	case VT_CY:
		return false;
	case VT_DECIMAL:
		if (byRef)
			return SUCCEEDED(VarDecFromR8(vx.asDouble(), va.pdecVal));
		else
			return SUCCEEDED(VarDecFromR8(vx.asDouble(), &va.decVal));
	case VT_DATE:
		return false;
    case VT_BSTR:
		if (byRef){
			UString str(vx.asString());
			*va.pbstrVal = SysAllocString(str);
		}else{
			UString str(vx.asString());
			va.bstrVal = SysAllocString(str);
		}
        return true;
	case VT_UNKNOWN:
		return false;
	case VT_DISPATCH:
		return false;
    default:
        return false;
    };
};

class ActiveXEvents : public IDispatch
{
private:
    DECLARE_OLE_UNKNOWN(ActiveXEvents);

    QActiveX	*m_activeX;
	IID			m_customId;
	bool		m_haveCustomId;
public:
    ActiveXEvents(QActiveX *ax) : m_activeX(ax), m_haveCustomId(false) {}
	ActiveXEvents(QActiveX *ax, REFIID iid) : m_activeX(ax), m_haveCustomId(true), m_customId(iid) {}
	virtual ~ActiveXEvents() 
    {
    }

	//IDispatch
	STDMETHODIMP GetIDsOfNames(REFIID, OLECHAR**, unsigned int, LCID, DISPID*)
	{ 
        return E_NOTIMPL;
    };

	STDMETHODIMP GetTypeInfo(unsigned int, LCID, ITypeInfo**)
	{ 
        return E_NOTIMPL;
    };

	STDMETHODIMP GetTypeInfoCount(unsigned int*)
	{ 
        return E_NOTIMPL;
    };

	STDMETHODIMP Invoke(DISPID dispIdMember, REFIID, LCID,
						  WORD wFlags, DISPPARAMS * pDispParams,
						  VARIANT*, EXCEPINFO *,
						  unsigned int *)
	{ 
	    if (wFlags & (DISPATCH_PROPERTYGET | DISPATCH_PROPERTYPUT | DISPATCH_PROPERTYPUTREF))
            return E_NOTIMPL;
        ASSERT(m_activeX);

        QActiveX::MemberIdMap::iterator mit = m_activeX->m_eventMemberIds.find((MEMBERID) dispIdMember);
        if (mit == m_activeX->m_eventMemberIds.end())
            return S_OK;
        int midx = mit->second;
        if (midx < 0 || midx >= m_activeX->getEventCount())
            return S_OK;
        QActiveX::FuncX &func = m_activeX->m_events[midx];

		vector<QVariant> params;
		vector<QString>	 names;
        if (pDispParams)
        {
            int nArg = min(func.params.size(), pDispParams->cArgs);
            for (int i = nArg - 1; i >= 0; i--)
            {
                VARIANTARG& va = pDispParams->rgvarg[i];
				QActiveX::ParamX &px = func.params[nArg - i - 1];
				names.push_back(px.name);

                QVariant vx;
                MSWVariantToVariant(va, vx);
                params.push_back(vx);
            };
        };
		m_activeX->dispatch(func.name, params, names);
		if (func.hasOut)
		{
            int nArg = min(func.params.size(), pDispParams->cArgs);
            for (int i = 0; i < nArg; i++)
            {
                VARIANTARG& va = pDispParams->rgvarg[i];
				QActiveX::ParamX &px = func.params[nArg - i - 1];
				if (px.IsOut())
				{
					QVariant& vx = params[nArg - i - 1];					
					VariantToMSWVariant(vx, va);
				};
			};
		}
    	return S_OK;
    }
};

bool ActiveXEventsInterface(ActiveXEvents *self, REFIID iid, void **_interface, const char *&desc)
{
    if (self->m_haveCustomId && IsEqualIID(iid, self->m_customId))
    {
    	*_interface = (IUnknown *) (IDispatch *) self;
    	desc = "Custom Dispatch Interface";
        return true;
    };

	return false;
};

DEFINE_OLE_TABLE(ActiveXEvents)
	OLE_IINTERFACE(IUnknown)
	OLE_INTERFACE(IID_IDispatch, IDispatch)
	OLE_INTERFACE_CUSTOM(ActiveXEventsInterface)
END_OLE_TABLE;

class FrameSite : 
    public IOleClientSite,
    public IOleInPlaceSiteEx,
    public IOleInPlaceFrame,
    public IOleItemContainer,
    public IDispatch,
    public IOleCommandTarget,
    public IOleDocumentSite,
    public IAdviseSink,
    public IOleControlSite,
	public IDocHostUIHandler
{
private:
    DECLARE_OLE_UNKNOWN(FrameSite);

public:
	FrameSite(QActiveX * win);
	virtual ~FrameSite();

	//IOleWindow
	STDMETHODIMP GetWindow(HWND*);
	STDMETHODIMP ContextSensitiveHelp(BOOL);

    //IOleInPlaceUIWindow
	STDMETHODIMP GetBorder(LPRECT);
	STDMETHODIMP RequestBorderSpace(LPCBORDERWIDTHS);
	STDMETHODIMP SetBorderSpace(LPCBORDERWIDTHS);
	STDMETHODIMP SetActiveObject(IOleInPlaceActiveObject*, LPCOLESTR);
	
    //IOleInPlaceFrame
	STDMETHODIMP InsertMenus(HMENU, LPOLEMENUGROUPWIDTHS);
	STDMETHODIMP SetMenu(HMENU, HOLEMENU, HWND);
	STDMETHODIMP RemoveMenus(HMENU);
	STDMETHODIMP SetStatusText(LPCOLESTR);
	STDMETHODIMP EnableModeless(BOOL);
	STDMETHODIMP TranslateAccelerator(LPMSG, WORD);

	//IOleInPlaceSite
	STDMETHODIMP CanInPlaceActivate();
	STDMETHODIMP OnInPlaceActivate();
	STDMETHODIMP OnUIActivate();
	STDMETHODIMP GetWindowContext(IOleInPlaceFrame**, IOleInPlaceUIWindow**, 
		LPRECT, LPRECT, LPOLEINPLACEFRAMEINFO);
	STDMETHODIMP Scroll(SIZE);
	STDMETHODIMP OnUIDeactivate(BOOL);
	STDMETHODIMP OnInPlaceDeactivate();
	STDMETHODIMP DiscardUndoState();
	STDMETHODIMP DeactivateAndUndo();
	STDMETHODIMP OnPosRectChange(LPCRECT);

    //IOleInPlaceSiteEx
	STDMETHODIMP OnInPlaceActivateEx(BOOL*, DWORD);
	STDMETHODIMP OnInPlaceDeactivateEx(BOOL);
	STDMETHODIMP RequestUIActivate();

	//IOleClientSite
	STDMETHODIMP SaveObject();
	STDMETHODIMP GetMoniker(DWORD, DWORD, IMoniker**);
	STDMETHODIMP GetContainer(LPOLECONTAINER FAR*);
	STDMETHODIMP ShowObject();
	STDMETHODIMP OnShowWindow(BOOL);
	STDMETHODIMP RequestNewObjectLayout();

	//IOleControlSite
	STDMETHODIMP OnControlInfoChanged();
	STDMETHODIMP LockInPlaceActive(BOOL);
	STDMETHODIMP GetExtendedControl(IDispatch**);
	STDMETHODIMP TransformCoords(POINTL*, POINTF*, DWORD);
	STDMETHODIMP TranslateAccelerator(LPMSG, DWORD);
	STDMETHODIMP OnFocus(BOOL);
	STDMETHODIMP ShowPropertyFrame();

	//IOleCommandTarget
	STDMETHODIMP QueryStatus(const GUID*, ULONG, OLECMD[], OLECMDTEXT*);
	STDMETHODIMP Exec(const GUID*, DWORD, DWORD, VARIANTARG*, VARIANTARG*);

	//IParseDisplayName
	STDMETHODIMP ParseDisplayName(IBindCtx*, LPOLESTR, ULONG*, IMoniker**);

    //IOleContainer
	STDMETHODIMP EnumObjects(DWORD, IEnumUnknown**);
	STDMETHODIMP LockContainer(BOOL);

	//IOleItemContainer
	STDMETHODIMP GetObject(LPOLESTR, DWORD, IBindCtx*, REFIID, void**);
	STDMETHODIMP GetObjectStorage(LPOLESTR, IBindCtx*, REFIID, void**);
	STDMETHODIMP IsRunning(LPOLESTR);
    
	//IDispatch
	STDMETHODIMP GetIDsOfNames(REFIID, OLECHAR**, unsigned int, LCID, DISPID*);
	STDMETHODIMP GetTypeInfo(unsigned int, LCID, ITypeInfo**);
	STDMETHODIMP GetTypeInfoCount(unsigned int*);
	STDMETHODIMP Invoke(DISPID, REFIID, LCID, WORD, DISPPARAMS*, VARIANT*, EXCEPINFO*, UINT*);

	//IAdviseSink
	void STDMETHODCALLTYPE OnDataChange(FORMATETC*, STGMEDIUM*);
	void STDMETHODCALLTYPE OnViewChange(DWORD, LONG);
	void STDMETHODCALLTYPE OnRename(IMoniker*);
	void STDMETHODCALLTYPE OnSave();
	void STDMETHODCALLTYPE OnClose();

    // IOleDocumentSite
	HRESULT STDMETHODCALLTYPE ActivateMe(IOleDocumentView __RPC_FAR *pViewToActivate);

	// IDocHostUIHandler
    HRESULT STDMETHODCALLTYPE ShowContextMenu(
            /* [in] */ DWORD dwID,
            /* [in] */ POINT __RPC_FAR *ppt,
            /* [in] */ IUnknown __RPC_FAR *pcmdtReserved,
            /* [in] */ IDispatch __RPC_FAR *pdispReserved);
        
    HRESULT STDMETHODCALLTYPE GetHostInfo( 
            /* [out][in] */ DOCHOSTUIINFO __RPC_FAR *pInfo);
        
    HRESULT STDMETHODCALLTYPE ShowUI( 
            /* [in] */ DWORD dwID,
            /* [in] */ IOleInPlaceActiveObject __RPC_FAR *pActiveObject,
            /* [in] */ IOleCommandTarget __RPC_FAR *pCommandTarget,
            /* [in] */ IOleInPlaceFrame __RPC_FAR *pFrame,
            /* [in] */ IOleInPlaceUIWindow __RPC_FAR *pDoc);
        
    HRESULT STDMETHODCALLTYPE HideUI( void);
        
    HRESULT STDMETHODCALLTYPE UpdateUI( void);
                
    HRESULT STDMETHODCALLTYPE OnDocWindowActivate( 
            /* [in] */ BOOL fActivate);
        
    HRESULT STDMETHODCALLTYPE OnFrameWindowActivate( 
            /* [in] */ BOOL fActivate);
        
    HRESULT STDMETHODCALLTYPE ResizeBorder( 
            /* [in] */ LPCRECT prcBorder,
            /* [in] */ IOleInPlaceUIWindow __RPC_FAR *pUIWindow,
            /* [in] */ BOOL fRameWindow);
        
    HRESULT STDMETHODCALLTYPE TranslateAccelerator( 
            /* [in] */ LPMSG lpMsg,
            /* [in] */ const GUID __RPC_FAR *pguidCmdGroup,
            /* [in] */ DWORD nCmdID);
        
    HRESULT STDMETHODCALLTYPE GetOptionKeyPath( 
            /* [out] */ LPOLESTR __RPC_FAR *pchKey,
            /* [in] */ DWORD dw);
        
    HRESULT STDMETHODCALLTYPE GetDropTarget( 
            /* [in] */ IDropTarget __RPC_FAR *pDropTarget,
            /* [out] */ IDropTarget __RPC_FAR *__RPC_FAR *ppDropTarget);
        
    HRESULT STDMETHODCALLTYPE GetExternal( 
            /* [out] */ IDispatch __RPC_FAR *__RPC_FAR *ppDispatch);
        
    HRESULT STDMETHODCALLTYPE TranslateUrl( 
            /* [in] */ DWORD dwTranslate,
            /* [in] */ OLECHAR __RPC_FAR *pchURLIn,
            /* [out] */ OLECHAR __RPC_FAR *__RPC_FAR *ppchURLOut);
        
    HRESULT STDMETHODCALLTYPE FilterDataObject( 
            /* [in] */ IDataObject __RPC_FAR *pDO,
            /* [out] */ IDataObject __RPC_FAR *__RPC_FAR *ppDORet);

protected:
	QActiveX * m_window;

	HDC  m_hDCBuffer;
	HWND m_hWndParent;

	bool m_bSupportsWindowlessActivation;
	bool m_bInPlaceLocked;
	bool m_bInPlaceActive;
	bool m_bUIActive;
	bool m_bWindowless;
    
	LCID m_nAmbientLocale;
	COLORREF m_clrAmbientForeColor;
	COLORREF m_clrAmbientBackColor;
	bool m_bAmbientShowHatching;
	bool m_bAmbientShowGrabHandles;
	bool m_bAmbientAppearance;
};

DEFINE_OLE_TABLE(FrameSite)
    OLE_INTERFACE(IID_IUnknown, IOleClientSite)

    OLE_IINTERFACE(IOleClientSite)

    OLE_INTERFACE(IID_IOleWindow, IOleInPlaceSite)
    OLE_IINTERFACE(IOleInPlaceSite)
    OLE_IINTERFACE(IOleInPlaceSiteEx)

    //OLE_IINTERFACE(IOleWindow)
    OLE_IINTERFACE(IOleInPlaceUIWindow)
    OLE_IINTERFACE(IOleInPlaceFrame)

    OLE_IINTERFACE(IParseDisplayName)
    OLE_IINTERFACE(IOleContainer)
    OLE_IINTERFACE(IOleItemContainer)

    OLE_IINTERFACE(IDispatch)

    OLE_IINTERFACE(IOleCommandTarget)

    OLE_IINTERFACE(IOleDocumentSite)

    OLE_IINTERFACE(IAdviseSink)

    OLE_IINTERFACE(IOleControlSite)
    OLE_IINTERFACE(IDocHostUIHandler)

END_OLE_TABLE;

QActiveX::QActiveX(QWidget *parent, const char *name, REFCLSID clsid)
: QWidget(parent, name)
{
    m_bAmbientUserMode = true;
    m_docAdviseCookie = 0;
    createActiveX(clsid);
}

QActiveX::~QActiveX()
{
	OleConnectionArray::iterator it = m_connections.begin();
	while (it != m_connections.end())
	{
		OleConnectionPoint& cp = it->first;
		cp->Unadvise(it->second);
		it++;
	};
	m_connections.clear();

    if (m_oleInPlaceObject.Ok()) 
	{
		m_oleInPlaceObject->InPlaceDeactivate();
		m_oleInPlaceObject->UIDeactivate();
	}

	if (m_oleObject.Ok()) 
	{
	    if (m_docAdviseCookie != 0)
    		m_oleObject->Unadvise(m_docAdviseCookie);

	    m_oleObject->DoVerb(OLEIVERB_HIDE, NULL, m_clientSite, 0, winId(), NULL);
        m_oleObject->Close(OLECLOSE_NOSAVE);
		m_oleObject->SetClientSite(NULL);
	}
}

void QActiveX::createActiveX(REFCLSID clsid)
{
	HRESULT hret;
    FrameSite *frame = new FrameSite(this);

    hret = m_clientSite.QueryInterface(IID_IOleClientSite, (IDispatch *)frame);
    ASSERT(SUCCEEDED(hret));
    AutoOleInterface<IAdviseSink> adviseSink(IID_IAdviseSink, (IDispatch *)frame);
    ASSERT(adviseSink.Ok());

    m_ActiveX.CreateInstance(clsid, IID_IUnknown);
	ASSERT(m_ActiveX.Ok());

	hret = m_Dispatch.QueryInterface(IID_IDispatch, m_ActiveX); 
	ASSERT(SUCCEEDED(hret));

	getTypeInfo();

	hret = m_oleObject.QueryInterface(IID_IOleObject, m_ActiveX); 
	ASSERT(SUCCEEDED(hret));

    hret = m_viewObject.QueryInterface(IID_IViewObject, m_ActiveX); 
	ASSERT(SUCCEEDED(hret));

    m_docAdviseCookie = 0;
    hret = m_oleObject->Advise(adviseSink, &m_docAdviseCookie);
	ASSERT(SUCCEEDED(hret));

    m_oleObject->SetHostNames(L"QActiveXContainer", NULL);
	OleSetContainedObject(m_oleObject, TRUE);
    OleRun(m_oleObject);

	hret = m_oleInPlaceObject.QueryInterface(IID_IOleInPlaceObject, m_ActiveX);
	ASSERT(SUCCEEDED(hret));

	DWORD dwMiscStatus;
	m_oleObject->GetMiscStatus(DVASPECT_CONTENT, &dwMiscStatus);
    ASSERT(SUCCEEDED(hret));

    if (dwMiscStatus & OLEMISC_SETCLIENTSITEFIRST)
		m_oleObject->SetClientSite(m_clientSite);

    AutoOleInterface<IPersistStreamInit>
		pPersistStreamInit(IID_IPersistStreamInit, m_oleObject);

    if (pPersistStreamInit.Ok())
    {
        hret = pPersistStreamInit->InitNew();
		ASSERT(SUCCEEDED(hret));
    };

    if (! (dwMiscStatus & OLEMISC_SETCLIENTSITEFIRST))
		m_oleObject->SetClientSite(m_clientSite);


	RECT posRect;
	posRect.left	= 0;
	posRect.top		= 0;
	posRect.right	= width();
	posRect.bottom	= height();

    m_oleObjectHWND = 0;

    if (m_oleInPlaceObject.Ok())
    {
    	hret = m_oleInPlaceObject->GetWindow(&m_oleObjectHWND);
        if (SUCCEEDED(hret))
	        ::SetActiveWindow(m_oleObjectHWND);
    };


    if (! (dwMiscStatus & OLEMISC_INVISIBLEATRUNTIME))
    {
		if (m_oleInPlaceObject.Ok())
			m_oleInPlaceObject->SetObjectRects(&posRect, &posRect);

		hret = m_oleObject->DoVerb(OLEIVERB_INPLACEACTIVATE, NULL, m_clientSite, 0, winId(), &posRect);
        hret = m_oleObject->DoVerb(OLEIVERB_SHOW, 0, m_clientSite, 0, winId(), &posRect);
    };

	if (! m_oleObjectHWND && m_oleInPlaceObject.Ok())
	{
		hret = m_oleInPlaceObject->GetWindow(&m_oleObjectHWND);
	};

	if (m_oleObjectHWND)
	{
		::SetActiveWindow(m_oleObjectHWND);
		::ShowWindow(m_oleObjectHWND, SW_SHOW);
	};
}

#define HIMETRIC_PER_INCH   2540
#define MAP_PIX_TO_LOGHIM(x,ppli)   MulDiv(HIMETRIC_PER_INCH, (x), (ppli))

static void PixelsToHimetric(SIZEL &sz)
{
	static int logX = 0;
	static int logY = 0;
	if (logY == 0)
	{
		HDC dc = GetDC(NULL);
		logX = GetDeviceCaps(dc, LOGPIXELSX);
		logY = GetDeviceCaps(dc, LOGPIXELSY);
		ReleaseDC(NULL, dc);
	};
#define HIMETRIC_INCH   2540
#define CONVERT(x, logpixels)   MulDiv(HIMETRIC_INCH, (x), (logpixels))
	sz.cx = CONVERT(sz.cx, logX);
	sz.cy = CONVERT(sz.cy, logY);
#undef CONVERT
#undef HIMETRIC_INCH
}

void QActiveX::resizeEvent(QResizeEvent *e)
{
	DEBUG_LOG("resize");
	RECT posRect;
	posRect.left = 0;
	posRect.top  = 0;
	posRect.right  = e->size().width();
	posRect.bottom = e->size().height();

    if (m_oleObject.Ok())
    {
        SIZEL sz = {e->size().width(), e->size().height()};
	    PixelsToHimetric(sz);
        SIZEL sz2;
        m_oleObject->GetExtent(DVASPECT_CONTENT, &sz2);
        if (sz2.cx !=  sz.cx || sz.cy != sz2.cy)
            m_oleObject->SetExtent(DVASPECT_CONTENT, &sz);
    };
    if (m_oleInPlaceObject.Ok()) 
		m_oleInPlaceObject->SetObjectRects(&posRect, &posRect);
}

void QActiveX::paintEvent(QPaintEvent*)
{
	DEBUG_LOG("paint");
	QPainter p(this);

	RECT posRect;
	posRect.left = 0;
	posRect.top = 0;
	posRect.right  = width();
	posRect.bottom = height();

	if (m_viewObject)
	{
		::RedrawWindow(m_oleObjectHWND, NULL, NULL, RDW_INTERNALPAINT);
		{
			RECTL *prcBounds = (RECTL *) &posRect;
			m_viewObject->Draw(DVASPECT_CONTENT, -1, NULL, NULL, NULL, 
				p.handle(), prcBounds, NULL, NULL, 0);
		}
	}
}

void QActiveX::mousePressEvent(QMouseEvent *e)
{
	UINT msg = 0;
	switch (e->button()){
	case LeftButton:
		msg = WM_LBUTTONDOWN;
		break;
	case RightButton:
		msg = WM_RBUTTONDOWN;
		break;
	case MidButton:
		msg = WM_MBUTTONDOWN;
		break;
	}
	mouseEvent(msg, e);
}

void QActiveX::mouseReleaseEvent(QMouseEvent *e)
{
	UINT msg = 0;
	switch (e->button()){
	case LeftButton:
		msg = WM_LBUTTONUP;
		break;
	case RightButton:
		msg = WM_RBUTTONUP;
		break;
	case MidButton:
		msg = WM_MBUTTONUP;
		break;
	}
	mouseEvent(msg, e);
}

void QActiveX::mouseDoubleClickEvent(QMouseEvent *e)
{
	UINT msg = 0;
	switch (e->button()){
	case LeftButton:
		msg = WM_LBUTTONDBLCLK;
		break;
	case RightButton:
		msg = WM_RBUTTONDBLCLK;
		break;
	case MidButton:
		msg = WM_MBUTTONDBLCLK;
		break;
	}
	mouseEvent(msg, e);
}

void QActiveX::mouseMoveEvent(QMouseEvent *e)
{
	mouseEvent(WM_MOUSEMOVE, e);
}

void QActiveX::mouseEvent(UINT msg, QMouseEvent *e)
{
	if (msg == NULL)
		return;
	WPARAM wParam = 0;
	LPARAM lParam = 0;
	if (e->state() & ControlButton) 
        wParam |= MK_CONTROL;
	if (e->state() & ShiftButton) 
        wParam |= MK_SHIFT;
	if (e->state() & AltButton) 
        wParam |= MK_ALT;
	if (e->state() & LeftButton) 
        wParam |= MK_LBUTTON;
	if (e->state() & MidButton) 
        wParam |= MK_MBUTTON;
	if (e->state() & RightButton) 
        wParam |= MK_RBUTTON;
	lParam  = (e->globalX() << 16) + e->globalY();
	::SendMessage(m_oleObjectHWND, msg, wParam, lParam);
}

void QActiveX::focusInEvent(QFocusEvent*)
{
	DEBUG_LOG("focus in");
	if (m_oleInPlaceActiveObject.Ok()) 
        m_oleInPlaceActiveObject->OnFrameWindowActivate(TRUE);
}

void QActiveX::focusOutEvent(QFocusEvent*)
{
	DEBUG_LOG("focus out");
	if (m_oleInPlaceActiveObject.Ok()) 
        m_oleInPlaceActiveObject->OnFrameWindowActivate(FALSE);
}

HRESULT QActiveX::ambientPropertyChanged(DISPID dispid)
{
    AutoOleInterface<IOleControl> oleControl(IID_IOleControl, m_oleObject);
    if (oleControl.Ok())
        return oleControl->OnAmbientPropertyChange(dispid);
    else
        return S_FALSE;
};

void QActiveX::dispatch(const QString&, vector<QVariant>&, const vector<QString>&)
{
}

QPopupMenu *QActiveX::createPopupMenu(const QPoint&)
{
	return NULL;
}

void QActiveX::getTypeInfo()
{
	HRESULT hret = 0;

	AutoOleInterface<IProvideClassInfo> classInfo(IID_IProvideClassInfo, m_ActiveX);
	if (! classInfo.Ok())
		return;

	AutoOleInterface<ITypeInfo> typeInfo;
	hret = classInfo->GetClassInfo(typeInfo.GetRef());
	if (! typeInfo.Ok())
		return;

	TYPEATTR *ta = NULL;
	hret = typeInfo->GetTypeAttr(&ta);
	if (! ta)
		return;

    ASSERT(ta->typekind == TKIND_COCLASS);

	for (int i = 0; i < ta->cImplTypes; i++)
	{
		HREFTYPE rt = 0;
		hret = typeInfo->GetRefTypeOfImplType(i, &rt);
		if (! SUCCEEDED(hret))
			continue;
		AutoOleInterface<ITypeInfo>  ti;
		hret = typeInfo->GetRefTypeInfo(rt, ti.GetRef());
		if (! ti.Ok())
			continue;
		bool defInterface = false;
        bool defEventSink = false;
        int impTypeFlags = 0;
        typeInfo->GetImplTypeFlags(i, &impTypeFlags);
        if (impTypeFlags & IMPLTYPEFLAG_FDEFAULT)
        {
            if (impTypeFlags & IMPLTYPEFLAG_FSOURCE)
            {
                defEventSink = true;
				if (impTypeFlags & IMPLTYPEFLAG_FDEFAULTVTABLE)
				{
					defEventSink = false;
				};
            }
            else
            {
				defInterface = true;
            }
        };
		getTypeInfo(ti, defInterface, defEventSink);
	};
    typeInfo->ReleaseTypeAttr(ta);
};

void ElemDescToParam(const ELEMDESC& ed, QActiveX::ParamX& param)
{
	param.flags = ed.idldesc.wIDLFlags;
	param.vt = ed.tdesc.vt;
    param.isPtr = (param.vt == VT_PTR);
    param.isSafeArray = (param.vt == VT_SAFEARRAY);
    if (param.isPtr || param.isSafeArray)
        param.vt = ed.tdesc.lptdesc->vt;
};

void QActiveX::getTypeInfo(ITypeInfo *ti, bool defInterface, bool defEventSink)
{
	ti->AddRef();
	AutoOleInterface<ITypeInfo> typeInfo(ti);

	TYPEATTR *ta = NULL;
	HRESULT hret = typeInfo->GetTypeAttr(&ta);
	if (! ta)
		return;

	if (ta->typekind == TKIND_DISPATCH)
	{
        if (defEventSink)
        {
            ActiveXEvents *disp = new ActiveXEvents(this, ta->guid);
            connectAdvise(ta->guid, disp);
        };
		for (int i = 0; i < ta->cFuncs; i++)
		{
			FUNCDESC FAR *fd = NULL;
			hret = typeInfo->GetFuncDesc(i, &fd);
			if (! fd)
				continue;
			BSTR anames[1] = {NULL};
			unsigned int n = 0;
			hret = typeInfo->GetNames(fd->memid, anames, 1, &n);
			if (anames[0])
			{
				QString name = CnvBSTR(anames[0]);
				SysFreeString(anames[0]);

                if (defInterface || defEventSink)
                {
                    FuncX func;
                    func.name = name;
                    func.memid = fd->memid;
					func.hasOut = false;

                    unsigned int maxPNames = fd->cParams + 1;
                    unsigned int nPNames = 0;
                    BSTR *pnames = new BSTR[maxPNames];

                    hret = typeInfo->GetNames(fd->memid, pnames, maxPNames, &nPNames);

					int pbase = 0;
					if (fd->cParams < int(nPNames))
					{
						pbase++;
	                    SysFreeString(pnames[0]);
					};

                    ElemDescToParam(fd->elemdescFunc, func.retType);
					for (int p = 0; p < fd->cParams; p++)
					{
						ParamX param;
						ElemDescToParam(fd->lprgelemdescParam[p], param);
                        param.name = CnvBSTR(pnames[pbase + p]);
                        SysFreeString(pnames[pbase + p]);
						func.hasOut |= (param.IsOut() || param.isPtr);
						func.params.push_back(param);
					};
                    delete [] pnames;

					if (defEventSink)
					{
						m_events.push_back(func);
                        m_eventMemberIds[fd->memid] = m_events.size() - 1;
					}
					else
					{
						if (fd->invkind == INVOKE_FUNC)
						{
                            m_methods.push_back(func);
							m_methodNames[func.name] = m_methods.size() - 1;
						}
						else
						{
							NameMap::iterator it = m_propNames.find(func.name);
                            int idx = -1;
							if (it == m_propNames.end())
                            {
                                m_props.push_back(PropX());
                                idx = m_props.size() - 1;
								m_propNames[func.name] = idx;
                                m_props[idx].name = func.name;
                                m_props[idx].memid = func.memid;
                                
                            }
                            else
                                idx = it->second;
							
							if (fd->invkind == INVOKE_PROPERTYGET)
								m_props[idx].type = func.retType;
							else if (func.params.size() > 0)
							{
								m_props[idx].arg = func.params[0];
								m_props[idx].putByRef = (fd->invkind == INVOKE_PROPERTYPUTREF);
							};
						};
					};
                };
			};
			typeInfo->ReleaseFuncDesc(fd);
		}
	}
	typeInfo->ReleaseTypeAttr(ta);
};

HRESULT QActiveX::connectAdvise(REFIID riid, IUnknown *events)
{
	OleConnectionPoint	cp;
	DWORD				adviseCookie = 0;

	AutoOleInterface<IConnectionPointContainer> cpContainer(IID_IConnectionPointContainer, m_ActiveX);
	if (! cpContainer.Ok())
		return E_FAIL;
	
	HRESULT hret = cpContainer->FindConnectionPoint(riid, cp.GetRef());
	if (! SUCCEEDED(hret))
		return hret;
	
	hret = cp->Advise(events, &adviseCookie);

	if (SUCCEEDED(hret))
		m_connections.push_back(OleConnection(cp, adviseCookie));
	return hret;
};

static IMalloc *iMalloc = NULL;

IMalloc *QOleInit::GetIMalloc()
{
	ASSERT(iMalloc);
	return iMalloc;
};

QOleInit::QOleInit()
{
    if (OleInitialize(NULL) == S_OK && iMalloc == NULL)
	    CoGetMalloc(1, &iMalloc);
    else if (iMalloc)
    	iMalloc->AddRef();
};

QOleInit::~QOleInit()
{
	if (iMalloc)
    {
    	if (iMalloc->Release() == 0)
        	iMalloc = NULL;
    };

    OleUninitialize();
}

FrameSite::FrameSite(QActiveX * win)
{
	m_window = win;
	m_bSupportsWindowlessActivation = true;
	m_bInPlaceLocked = false;
	m_bUIActive = false;
	m_bInPlaceActive = false;
	m_bWindowless = false;

	m_nAmbientLocale = 0;
	m_clrAmbientForeColor = ::GetSysColor(COLOR_WINDOWTEXT);
	m_clrAmbientBackColor = ::GetSysColor(COLOR_WINDOW);
	m_bAmbientShowHatching = true;
	m_bAmbientShowGrabHandles = true;
	m_bAmbientAppearance = true;
 
	m_hDCBuffer = NULL;
	m_hWndParent = m_window->winId();
}

FrameSite::~FrameSite()
{
}

//IDispatch

HRESULT FrameSite::GetIDsOfNames(REFIID, OLECHAR **, unsigned int,
								 LCID, DISPID*)
{
	DEBUG_LOG("FrameSite::GetIDsOfNames");
	return E_NOTIMPL;
}

HRESULT FrameSite::GetTypeInfo(unsigned int, LCID, ITypeInfo**)
{
	DEBUG_LOG("FrameSite::GetTypeInfo");
	return E_NOTIMPL;
}

HRESULT FrameSite::GetTypeInfoCount(unsigned int*)
{
	DEBUG_LOG("FrameSite::GetTypeInfoCount");
	return E_NOTIMPL;
}

HRESULT FrameSite::Invoke(DISPID dispIdMember, REFIID, LCID,
						  WORD wFlags, DISPPARAMS*,
						  VARIANT * pVarResult, EXCEPINFO*,
						  unsigned int*)
{
	DEBUG_LOG("FrameSite::Invoke");
	if (!(wFlags & DISPATCH_PROPERTYGET))
        return S_OK;

	if (pVarResult == NULL) 
		return E_INVALIDARG;

    V_VT(pVarResult) = VT_BOOL;
	switch (dispIdMember)
	{
        case DISPID_AMBIENT_MESSAGEREFLECT:
            V_BOOL(pVarResult)= FALSE;
            return S_OK;

        case DISPID_AMBIENT_DISPLAYASDEFAULT:
            V_BOOL(pVarResult)= TRUE;
            return S_OK;

        case DISPID_AMBIENT_OFFLINEIFNOTCONNECTED:
            V_BOOL(pVarResult) = TRUE;
            return S_OK;

        case DISPID_AMBIENT_SILENT:
            V_BOOL(pVarResult)= TRUE;
            return S_OK;

		case DISPID_AMBIENT_APPEARANCE:
			pVarResult->vt = VT_BOOL;
			pVarResult->boolVal = m_bAmbientAppearance;
			break;

		case DISPID_AMBIENT_FORECOLOR:
			pVarResult->vt = VT_I4;
			pVarResult->lVal = (long) m_clrAmbientForeColor;
			break;

		case DISPID_AMBIENT_BACKCOLOR:
			pVarResult->vt = VT_I4;
			pVarResult->lVal = (long) m_clrAmbientBackColor;
			break;

		case DISPID_AMBIENT_LOCALEID:
			pVarResult->vt = VT_I4;
			pVarResult->lVal = (long) m_nAmbientLocale;
			break;

		case DISPID_AMBIENT_USERMODE:
			pVarResult->vt = VT_BOOL;
			pVarResult->boolVal = m_window->m_bAmbientUserMode;
			break;

		case DISPID_AMBIENT_SHOWGRABHANDLES:
			pVarResult->vt = VT_BOOL;
			pVarResult->boolVal = m_bAmbientShowGrabHandles;
			break;

		case DISPID_AMBIENT_SHOWHATCHING:
			pVarResult->vt = VT_BOOL;
			pVarResult->boolVal = m_bAmbientShowHatching;
			break;

		default:
			return DISP_E_MEMBERNOTFOUND;
	}
    return S_OK;
}

//IOleWindow

HRESULT FrameSite::GetWindow(HWND * phwnd)
{
	DEBUG_LOG("FrameSite::GetWindow");
	if (phwnd == NULL) 
        return E_INVALIDARG;
	(*phwnd) = m_hWndParent;
	return S_OK;
}

HRESULT FrameSite::ContextSensitiveHelp(BOOL)
{
	DEBUG_LOG("FrameSite::ContextSensitiveHelp");
	return S_OK;
}

//IOleInPlaceUIWindow

HRESULT FrameSite::GetBorder(LPRECT lprectBorder)
{
	DEBUG_LOG("FrameSite::GetBorder");
	if (lprectBorder == NULL) 
        return E_INVALIDARG;
	return INPLACE_E_NOTOOLSPACE;
}

HRESULT FrameSite::RequestBorderSpace(LPCBORDERWIDTHS pborderwidths)
{
	DEBUG_LOG("FrameSite::RequestBorderSpace");
	if (pborderwidths == NULL) 
        return E_INVALIDARG;
	return INPLACE_E_NOTOOLSPACE;
}

HRESULT FrameSite::SetBorderSpace(LPCBORDERWIDTHS)
{
	DEBUG_LOG("FrameSite::SetBorderSpace");
	return S_OK;
}

HRESULT FrameSite::SetActiveObject(IOleInPlaceActiveObject *pActiveObject, LPCOLESTR)
{
	DEBUG_LOG("FrameSite::SetActiveObject");
    if (pActiveObject)
        pActiveObject->AddRef();
    m_window->m_oleInPlaceActiveObject = pActiveObject;
	return S_OK;
}

//IOleInPlaceFrame

HRESULT FrameSite::InsertMenus(HMENU, LPOLEMENUGROUPWIDTHS)
{
	DEBUG_LOG("FrameSite::InsertMenus");
	return S_OK;
}

HRESULT FrameSite::SetMenu(HMENU, HOLEMENU, HWND)
{
	DEBUG_LOG("FrameSite::SetMenu");
	return S_OK;
}

HRESULT FrameSite::RemoveMenus(HMENU)
{
	DEBUG_LOG("FrameSite::RemoveMenus");
	return S_OK;
}

HRESULT FrameSite::SetStatusText(LPCOLESTR)
{
	DEBUG_LOG("FrameSite::SetStatusText");
	return S_OK;
}

HRESULT FrameSite::EnableModeless(BOOL)
{
	DEBUG_LOG("FrameSite::EnableModeless");
	return S_OK;
}

HRESULT FrameSite::TranslateAccelerator(LPMSG lpmsg, WORD)
{
	DEBUG_LOG("FrameSite::TranslateAccelerator");
    if (m_window->m_oleInPlaceActiveObject.Ok())
    	m_window->m_oleInPlaceActiveObject->TranslateAccelerator(lpmsg);
	return S_FALSE;
}

//IOleInPlaceSite

HRESULT FrameSite::CanInPlaceActivate()
{
	DEBUG_LOG("FrameSite::CanInPlaceActivate");
	return S_OK;
}

HRESULT FrameSite::OnInPlaceActivate()
{
	DEBUG_LOG("FrameSite::OnInPlaceActivate");
	m_bInPlaceActive = true;
	return S_OK;
}

HRESULT FrameSite::OnUIActivate()
{
	DEBUG_LOG("FrameSite::OnUIActivate");
	m_bUIActive = true;
	return S_OK;
}

HRESULT FrameSite::GetWindowContext(IOleInPlaceFrame **ppFrame,
									IOleInPlaceUIWindow **ppDoc,
									LPRECT lprcPosRect,
									LPRECT lprcClipRect,
									LPOLEINPLACEFRAMEINFO lpFrameInfo)
{
	DEBUG_LOG("FrameSite::GetWindowContext");
	if (ppFrame == NULL || ppDoc == NULL || lprcPosRect == NULL ||
		lprcClipRect == NULL || lpFrameInfo == NULL)
	{
		if (ppFrame != NULL) 
            (*ppFrame) = NULL;
		if (ppDoc != NULL) 
            (*ppDoc) = NULL;
		return E_INVALIDARG;
	}

    HRESULT hr = QueryInterface(IID_IOleInPlaceFrame, (void **) ppFrame);
    if (! SUCCEEDED(hr))
        return E_UNEXPECTED;

    hr = QueryInterface(IID_IOleInPlaceUIWindow, (void **) ppDoc);
    if (! SUCCEEDED(hr))
    {
        (*ppFrame)->Release();
        *ppFrame = NULL;
        return E_UNEXPECTED;
    };

    if (lprcPosRect)
    {
	    lprcPosRect->left = lprcPosRect->top = 0;
	    lprcPosRect->right  = m_window->width();
	    lprcPosRect->bottom = m_window->height();
    };
    if (lprcClipRect)
    {
	    lprcClipRect->left = lprcClipRect->top = 0;
	    lprcClipRect->right  = m_window->width();
	    lprcClipRect->bottom = m_window->height();
    };

    memset(lpFrameInfo, 0, sizeof(OLEINPLACEFRAMEINFO));
    lpFrameInfo->cb = sizeof(OLEINPLACEFRAMEINFO);
	lpFrameInfo->hwndFrame = m_hWndParent;
	return S_OK;
}

HRESULT FrameSite::Scroll(SIZE)
{
	DEBUG_LOG("FrameSite::Scroll");
	return S_OK;
}

HRESULT FrameSite::OnUIDeactivate(BOOL)
{
	DEBUG_LOG("FrameSite::OnUIDeactivate");
	m_bUIActive = false;
	return S_OK;
}

HRESULT FrameSite::OnInPlaceDeactivate()
{
	DEBUG_LOG("FrameSite::OnInPlaceDeactivate");
	m_bInPlaceActive = false;
	return S_OK;
}

HRESULT FrameSite::DiscardUndoState()
{
	DEBUG_LOG("FrameSite::DiscardUndoState");
	return S_OK;
}

HRESULT FrameSite::DeactivateAndUndo()
{
	DEBUG_LOG("FrameSite::DeactivateAndUndo");
	return S_OK;
}

HRESULT FrameSite::OnPosRectChange(LPCRECT lprcPosRect)
{
	DEBUG_LOG("FrameSite::OnPosRectChange");
    if (m_window->m_oleInPlaceObject.Ok() && lprcPosRect)
        m_window->m_oleInPlaceObject->SetObjectRects(lprcPosRect, lprcPosRect);
	return S_OK;
}

//IOleInPlaceSiteEx

HRESULT FrameSite::OnInPlaceActivateEx(BOOL * pfNoRedraw, DWORD)
{
	DEBUG_LOG("FrameSite::OnInPlaceActivateEx");
	OleLockRunning(m_window->m_ActiveX, TRUE, FALSE);
    if (pfNoRedraw) 
        (*pfNoRedraw) = FALSE;
	return S_OK;
}

HRESULT FrameSite::OnInPlaceDeactivateEx(BOOL)
{
	DEBUG_LOG("FrameSite::OnInPlaceDeactivateEx");
    OleLockRunning(m_window->m_ActiveX, FALSE, FALSE);
	return S_OK;
}

HRESULT FrameSite::RequestUIActivate()
{
	DEBUG_LOG("FrameSite::RequestUIActivate");
	return S_OK;
}

//IOleClientSite

HRESULT FrameSite::SaveObject()
{
	DEBUG_LOG("FrameSite::SaveObject");
	return S_OK;
}

HRESULT FrameSite::GetMoniker(DWORD, DWORD, IMoniker ** ppmk)
{
	DEBUG_LOG("FrameSite::GetMoniker");
    if (! ppmk)
        return E_FAIL;
    *ppmk = NULL;
	return E_FAIL ;
}

HRESULT FrameSite::GetContainer(LPOLECONTAINER * ppContainer)
{
	DEBUG_LOG("FrameSite::GetContainer");
	if (ppContainer == NULL) 
        return E_INVALIDARG;
    HRESULT hr = QueryInterface(IID_IOleContainer, (void**)(ppContainer));
    ASSERT(SUCCEEDED(hr));
	return hr;
}

HRESULT FrameSite::ShowObject()
{
	DEBUG_LOG("FrameSite::ShowObject");
	if (m_window->m_oleObjectHWND)
		::ShowWindow(m_window->m_oleObjectHWND, SW_SHOW);
	return S_OK;
}

HRESULT FrameSite::OnShowWindow(BOOL)
{
	DEBUG_LOG("FrameSite::OnShowWindow");
	return S_OK;
}

HRESULT FrameSite::RequestNewObjectLayout()
{
	DEBUG_LOG("FrameSite::RequestNewObjectLayout");
	return E_NOTIMPL;
}

// IParseDisplayName

HRESULT FrameSite::ParseDisplayName(IBindCtx*, LPOLESTR, ULONG*, IMoniker**)
{
	DEBUG_LOG("FrameSite::ParseDisplayName");
	return E_NOTIMPL;
}

//IOleContainer

HRESULT FrameSite::EnumObjects(DWORD, IEnumUnknown**)
{
	DEBUG_LOG("FrameSite::EnumObjects");
	return E_NOTIMPL;
}

HRESULT FrameSite::LockContainer(BOOL)
{
	DEBUG_LOG("FrameSite::LockContainer");
	return S_OK;
}

//IOleItemContainer

HRESULT FrameSite::GetObject(LPOLESTR pszItem, DWORD, 
							 IBindCtx*, REFIID, void ** ppvObject)
{
	DEBUG_LOG("FrameSite::GetObject");
	if (pszItem == NULL) 
        return E_INVALIDARG;
	if (ppvObject == NULL) 
        return E_INVALIDARG;
	*ppvObject = NULL;
	return MK_E_NOOBJECT;
}

HRESULT FrameSite::GetObjectStorage(LPOLESTR pszItem, IBindCtx*, 
									REFIID, void ** ppvStorage)
{
	DEBUG_LOG("FrameSite::GetObjectStorage");
	if (pszItem == NULL) 
        return E_INVALIDARG;
	if (ppvStorage == NULL) 
        return E_INVALIDARG;
	*ppvStorage = NULL;
	return MK_E_NOOBJECT;
}

HRESULT FrameSite::IsRunning(LPOLESTR pszItem)
{
	DEBUG_LOG("FrameSite::IsRunning");
	if (pszItem == NULL) 
        return E_INVALIDARG;
	return MK_E_NOOBJECT;
}

//IOleControlSite

HRESULT FrameSite::OnControlInfoChanged()
{
	DEBUG_LOG("FrameSite::OnControlInfoChanged");
	return S_OK;
}

HRESULT FrameSite::LockInPlaceActive(BOOL fLock)
{
	DEBUG_LOG("FrameSite::LockInPlaceActive");
	m_bInPlaceLocked = (fLock) ? true : false;
	return S_OK;
}

HRESULT FrameSite::GetExtendedControl(IDispatch**)
{
	DEBUG_LOG("FrameSite::GetExtendedControl");
	return E_NOTIMPL;
}

HRESULT FrameSite::TransformCoords(POINTL * pPtlHimetric, POINTF * pPtfContainer, DWORD)
{
	DEBUG_LOG("FrameSite::TransformCoords");
	if (pPtlHimetric == NULL)
		return E_INVALIDARG;
	if (pPtfContainer == NULL)
		return E_INVALIDARG;
	return E_NOTIMPL;
}

HRESULT FrameSite::TranslateAccelerator(LPMSG, DWORD)
{
	DEBUG_LOG("FrameSite::TranslateAccelerator");
	return E_NOTIMPL;
}

HRESULT FrameSite::OnFocus(BOOL)
{
	DEBUG_LOG("FrameSite::OnFocus");
	return S_OK;
}

HRESULT FrameSite::ShowPropertyFrame()
{
	DEBUG_LOG("FrameSite::ShowPropertyFrame");
	return E_NOTIMPL;
}

//IOleCommandTarget

HRESULT FrameSite::QueryStatus(const GUID*, ULONG cCmds, 
							   OLECMD * prgCmds, OLECMDTEXT*)
{
	DEBUG_LOG("FrameSite::QueryStatus");
	if (prgCmds == NULL) return E_INVALIDARG;
	bool bCmdGroupFound = false;
	for (ULONG nCmd = 0; nCmd < cCmds; nCmd++)
	{
		prgCmds[nCmd].cmdf = 0;
	}
	if (!bCmdGroupFound) { OLECMDERR_E_UNKNOWNGROUP; }
	return S_OK;
}

HRESULT FrameSite::Exec(const GUID*, DWORD, 
						DWORD, VARIANTARG*, 
						VARIANTARG*)
{
	DEBUG_LOG("FrameSite::Exec");
	bool bCmdGroupFound = false;
	if (!bCmdGroupFound)
		return OLECMDERR_E_UNKNOWNGROUP;
	return OLECMDERR_E_NOTSUPPORTED;
}

//IAdviseSink

void STDMETHODCALLTYPE FrameSite::OnDataChange(FORMATETC*, STGMEDIUM*)
{
	DEBUG_LOG("FrameSite::OnDataChange");
}

void STDMETHODCALLTYPE FrameSite::OnViewChange(DWORD, LONG)
{
	DEBUG_LOG("FrameSite::OnViewChange");
}

void STDMETHODCALLTYPE FrameSite::OnRename(IMoniker*)
{
	DEBUG_LOG("FrameSite::OnRename");
}

void STDMETHODCALLTYPE FrameSite::OnSave()
{
	DEBUG_LOG("FrameSite::OnSave");
}

void STDMETHODCALLTYPE FrameSite::OnClose()
{
	DEBUG_LOG("FrameSite::OnClose");
}

// IOleDocumentSite
HRESULT STDMETHODCALLTYPE FrameSite::ActivateMe(
        /* [in] */ IOleDocumentView __RPC_FAR *pViewToActivate)
{
	DEBUG_LOG("FrameSite::ActivateMe");
    AutoOleInterface<IOleInPlaceSite> inPlaceSite(IID_IOleInPlaceSite, (IDispatch *) this);
    if (!inPlaceSite.Ok())
    	return E_FAIL;

    if (pViewToActivate)
    {
    	m_window->m_docView = pViewToActivate;
        m_window->m_docView->SetInPlaceSite(inPlaceSite);
    }
    else
    {
    	AutoOleInterface<IOleDocument> oleDoc(IID_IOleDocument, m_window->m_oleObject);
        if (! oleDoc.Ok())
        	return E_FAIL;

        HRESULT hr = oleDoc->CreateView(inPlaceSite, NULL, 0, m_window->m_docView.GetRef());
        if (hr != S_OK)
        	return E_FAIL;

		m_window->m_docView->SetInPlaceSite(inPlaceSite);
    };
    m_window->m_docView->UIActivate(TRUE);
    return S_OK;
};

HRESULT STDMETHODCALLTYPE FrameSite::ShowContextMenu(
            /* [in] */ DWORD,
            /* [in] */ POINT __RPC_FAR*,
            /* [in] */ IUnknown __RPC_FAR*,
            /* [in] */ IDispatch __RPC_FAR*)
{
	DEBUG_LOG("FrameSite::ShowContextMenu");
	return S_OK;
}
        
HRESULT STDMETHODCALLTYPE FrameSite::GetHostInfo( 
            /* [out][in] */ DOCHOSTUIINFO __RPC_FAR*)
{
	DEBUG_LOG("FrameSite::GetHostInfo");
	return S_OK;
}
        
HRESULT STDMETHODCALLTYPE FrameSite::ShowUI( 
            /* [in] */ DWORD,
            /* [in] */ IOleInPlaceActiveObject __RPC_FAR*,
            /* [in] */ IOleCommandTarget __RPC_FAR*,
            /* [in] */ IOleInPlaceFrame __RPC_FAR*,
            /* [in] */ IOleInPlaceUIWindow __RPC_FAR*)
{
	DEBUG_LOG("FrameSite::ShowUI");
	return S_OK;
}
        
HRESULT STDMETHODCALLTYPE FrameSite::HideUI( void)
{
	DEBUG_LOG("FrameSite::HideUI");
	return S_OK;
}
        
HRESULT STDMETHODCALLTYPE FrameSite::UpdateUI( void)
{
	DEBUG_LOG("FrameSite::UpdateUI");
	return S_OK;
}
                
HRESULT STDMETHODCALLTYPE FrameSite::OnDocWindowActivate( 
            /* [in] */ BOOL)
{
	DEBUG_LOG("FrameSite::OnDocWindowActivate");
	return S_OK;
}
        
HRESULT STDMETHODCALLTYPE FrameSite::OnFrameWindowActivate( 
            /* [in] */ BOOL)
{
	DEBUG_LOG("FrameSite::OnFrameWindowActivate");
	return S_OK;
}
        
HRESULT STDMETHODCALLTYPE FrameSite::ResizeBorder( 
            /* [in] */ LPCRECT,
            /* [in] */ IOleInPlaceUIWindow __RPC_FAR*,
            /* [in] */ BOOL)
{
	DEBUG_LOG("FrameSite::ResizeBorder");
	return S_OK;
}
        
HRESULT STDMETHODCALLTYPE FrameSite::TranslateAccelerator( 
            /* [in] */ LPMSG,
            /* [in] */ const GUID __RPC_FAR*,
            /* [in] */ DWORD)
{
	DEBUG_LOG("FrameSite::TranslateAccelerator");
	return S_OK;
}
        
HRESULT STDMETHODCALLTYPE FrameSite::GetOptionKeyPath( 
            /* [out] */ LPOLESTR __RPC_FAR*,
            /* [in] */ DWORD)
{
	DEBUG_LOG("FrameSite::GetOptionKeyPath");
	return S_OK;
}
        
HRESULT STDMETHODCALLTYPE FrameSite::GetDropTarget( 
            /* [in] */ IDropTarget __RPC_FAR*,
            /* [out] */ IDropTarget __RPC_FAR *__RPC_FAR*)
{
	DEBUG_LOG("FrameSite::GetDropTarget");
	return E_NOTIMPL;
}
        
HRESULT STDMETHODCALLTYPE FrameSite::GetExternal( 
            /* [out] */ IDispatch __RPC_FAR *__RPC_FAR*)
{
	DEBUG_LOG("FrameSite::GetExternal");
	return S_OK;
}
        
HRESULT STDMETHODCALLTYPE FrameSite::TranslateUrl( 
            /* [in] */ DWORD,
            /* [in] */ OLECHAR __RPC_FAR*,
            /* [out] */ OLECHAR __RPC_FAR *__RPC_FAR*)
{
	DEBUG_LOG("FrameSite::TranslateUrl");
	return S_OK;
}
        
HRESULT STDMETHODCALLTYPE FrameSite::FilterDataObject( 
            /* [in] */ IDataObject __RPC_FAR*,
            /* [out] */ IDataObject __RPC_FAR *__RPC_FAR*)
{
	DEBUG_LOG("FrameSite::FilterDataObject");
	return S_OK;
}

