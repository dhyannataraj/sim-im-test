/***************************************************************************
                          icons.cpp  -  description
                             -------------------
    begin                : Sun Mar 17 2002
    copyright            : (C) 2002 by Vladimir Shutoff
    email                : vovan@shutoff.ru
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "icons.h"
#include "simapi.h"
#include "core.h"
#include "textshow.h"

#include <qiconset.h>
#include <qmime.h>
#include <qimage.h>
#include <qpainter.h>
#include <qbitmap.h>
#include <qdragobject.h>
#include <qfile.h>
#include <qthread.h>
#include <qbuffer.h>

#ifdef USE_KDE
#include <kapp.h>
#include <kipc.h>
#include <kiconloader.h>
#endif

#include <map>
using namespace std;

#include "xpm/exit.xpm"
#include "xpm/button_ok.xpm"
#include "xpm/button_cancel.xpm"
#include "xpm/1downarrow.xpm"
#include "xpm/1uparrow.xpm"
#include "xpm/1leftarrow.xpm"
#include "xpm/1rightarrow.xpm"
#include "xpm/configure.xpm"
#include "xpm/fileopen.xpm"
#include "xpm/filesave.xpm"
#include "xpm/collapsed.xpm"
#include "xpm/expanded.xpm"
#include "xpm/remove.xpm"
#include "xpm/error.xpm"
#include "xpm/mail_generic.xpm"
#include "xpm/info.xpm"
#include "xpm/text.xpm"
#include "xpm/phone.xpm"
#include "xpm/fax.xpm"
#include "xpm/cell.xpm"
#include "xpm/pager.xpm"
#include "xpm/webpress.xpm"
#include "xpm/nophone.xpm"
#include "xpm/find.xpm"
#include "xpm/nonim.xpm"
#include "xpm/online_on.xpm"
#include "xpm/online_off.xpm"
#include "xpm/grp_on.xpm"
#include "xpm/grp_off.xpm"
#include "xpm/grp_create.xpm"
#include "xpm/grp_rename.xpm"
#include "xpm/home.xpm"
#include "xpm/work.xpm"
#include "xpm/security.xpm"
#include "xpm/run.xpm"
#include "xpm/network.xpm"
#include "xpm/message.xpm"
#include "xpm/file.xpm"
#include "xpm/sms.xpm"
#include "xpm/url.xpm"
#include "xpm/contacts.xpm"
#include "xpm/auth.xpm"
#include "xpm/empty.xpm"
#include "xpm/translit.xpm"
#include "xpm/bgcolor.xpm"
#include "xpm/fgcolor.xpm"
#include "xpm/text_bold.xpm"
#include "xpm/text_italic.xpm"
#include "xpm/text_under.xpm"
#include "xpm/text_strike.xpm"
#include "xpm/cancel.xpm"
#include "xpm/filter.xpm"
#include "xpm/history.xpm"
#include "xpm/editcut.xpm"
#include "xpm/editcopy.xpm"
#include "xpm/editpaste.xpm"
#include "xpm/undo.xpm"
#include "xpm/redo.xpm"
#include "xpm/typing.xpm"
#include "xpm/pict.xpm"
#include "xpm/fileclose.xpm"
#include "xpm/encrypted.xpm"
#include "xpm/smile0.xpm"
#include "xpm/smile1.xpm"
#include "xpm/smile2.xpm"
#include "xpm/smile3.xpm"
#include "xpm/smile4.xpm"
#include "xpm/smile5.xpm"
#include "xpm/smile6.xpm"
#include "xpm/smile7.xpm"
#include "xpm/smile8.xpm"
#include "xpm/smile9.xpm"
#include "xpm/smileA.xpm"
#include "xpm/smileB.xpm"
#include "xpm/smileC.xpm"
#include "xpm/smileD.xpm"
#include "xpm/smileE.xpm"
#include "xpm/smileF.xpm"
#include "xpm/listmsg.xpm"
#include "xpm/urgentmsg.xpm"
#include "xpm/help.xpm"
#include "xpm/more.xpm"
#include "xpm/away.xpm"
#include "xpm/na.xpm"
#include "xpm/dnd.xpm"
#include "xpm/occupied.xpm"
#include "xpm/ffc.xpm"
#include "xpm/icq.xpm"
#include "xpm/msn.xpm"
#include "xpm/aim.xpm"
#include "xpm/yahoo.xpm"
#include "xpm/yahoo_offline.xpm"
#include "xpm/yahoo_away.xpm"
#include "xpm/yahoo_dnd.xpm"
#include "xpm/chat.xpm"
#include "xpm/encoding.xpm"
#include "xpm/add.xpm"
#include "xpm/new.xpm"

#define KICON(A)    addIcon(#A, p_##A, #A, 0);
#define ICON(A)		addIcon(#A, A, NULL, 0);

#ifdef USE_IE

#pragma warning(disable: 4250)

class CMultiThreaded
{
protected:
	STDMETHODIMP_(ULONG) Increment(long &reflong)  
	{
		return InterlockedIncrement(&reflong);
	}

	STDMETHODIMP_(ULONG) Decrement(long &reflong)
	{
		return InterlockedDecrement(&reflong); 
	}
};

class CObjRoot
{
protected:
	long	m_cRef;
protected:
	STDMETHOD_(ULONG,_AddRef)() =0;
	STDMETHOD_(ULONG,_Release)() =0;
public:
	CObjRoot() : m_cRef(1) 
	{
	}
	virtual ~CObjRoot() {}
	static long *p_ObjCount;
};

long * CObjRoot::p_ObjCount = NULL;

template <class Interface>
class InterfaceImpl: public virtual CObjRoot, public Interface 
{
public:
	STDMETHOD(QueryInterface)(REFIID riid,LPVOID *ppv) = 0; 
	STDMETHODIMP_(ULONG) AddRef() 
	{
		return _AddRef(); 
	}
	STDMETHODIMP_(ULONG) Release() 
	{
		return _Release();
	}
};

template <class ThreadModel = CMultiThreaded>
class CComBase  : public virtual CObjRoot ,  public ThreadModel
{
public:
	CComBase() {};
	virtual ~CComBase() {};
protected:
	STDMETHODIMP_(ULONG) _AddRef() 
	{
		if(p_ObjCount)
			ThreadModel::Increment(*p_ObjCount); 
		return ThreadModel::Increment(m_cRef); 
	}
	STDMETHODIMP_(ULONG) _Release() 
	{
		long Value = ThreadModel::Decrement(m_cRef); 
		if(!m_cRef)
			delete this;
		if(p_ObjCount)
			ThreadModel::Decrement(*p_ObjCount); 
		return Value;
	}
};

template<class comObj>
class CMultiCreator
{
protected:
	CMultiCreator():m_pObj(0) {};
	comObj *CreateObject()
	{
		return new comObj;
	}
	comObj * m_pObj;
};

template <class comObj, class creatorClass  = CMultiCreator < comObj > >
class CClassFactory :  public CComBase<>, public InterfaceImpl<IClassFactory>, public creatorClass 
{
public:
	CClassFactory() {};
	virtual ~CClassFactory() {};

	STDMETHOD(QueryInterface)(REFIID riid,LPVOID *ppv)
	{
		*ppv = NULL;
		if(IsEqualIID(riid,IID_IUnknown) || IsEqualIID(riid,IID_IClassFactory))
		{
			*ppv = (IClassFactory *) this;
			_AddRef();
			return S_OK;
		}
		return E_NOINTERFACE;
	}

	STDMETHODIMP CreateInstance(LPUNKNOWN pUnkOuter, REFIID riid, LPVOID *ppvObj)
	{
		*ppvObj = NULL;
		if (pUnkOuter)
    		return CLASS_E_NOAGGREGATION;
		m_pObj = CreateObject();  // m_pObj is defined in creatorClass 
		if (!m_pObj)
    		return E_OUTOFMEMORY;
		HRESULT hr = m_pObj->QueryInterface(riid, ppvObj);
		if (hr != S_OK)
			delete m_pObj;
		return hr;
	}

	STDMETHODIMP LockServer(BOOL) {	return S_OK; }  // not implemented
};

class PluginProtocol;

class PluginProtocolThread : public QThread
{
public:
	PluginProtocolThread() {}
protected:
	void run();
};

class PluginProtocol :  public CComBase<>, 
	public InterfaceImpl<IInternetProtocol>
{
public:
	PluginProtocol();
	virtual ~PluginProtocol();
	STDMETHOD(QueryInterface)(REFIID riid,LPVOID *ppv); 
	static	void	Register();
	static	void	Unregister();
protected:
	static	IInternetSession		*s_session;
	static	IClassFactory			*s_cf;	
	static	PluginProtocolThread	*s_thread;
	static	list<PluginProtocol*>	*s_protocols;
	static	QMutex					*s_mutex;
	static	QWaitCondition			*s_condition;

	void	Process();
	void	Close();
	void	SetResult(HRESULT result);
	void	SetState(DWORD state, wchar_t *data);
	void	SetData(DWORD state);

	QBuffer					*m_data;
	unsigned				m_data_pos;
	IInternetProtocolSink	*m_pOIProtSink;
	IInternetBindInfo		*m_pOIBindInfo;

		virtual HRESULT STDMETHODCALLTYPE Start( 
            /* [in] */ LPCWSTR szUrl,
            /* [in] */ IInternetProtocolSink __RPC_FAR *pOIProtSink,
            /* [in] */ IInternetBindInfo __RPC_FAR *pOIBindInfo,
            /* [in] */ DWORD grfPI,
            /* [in] */ DWORD dwReserved);
        
        virtual HRESULT STDMETHODCALLTYPE Continue( 
            /* [in] */ PROTOCOLDATA __RPC_FAR *pProtocolData);
        
        virtual HRESULT STDMETHODCALLTYPE Abort( 
            /* [in] */ HRESULT hrReason,
            /* [in] */ DWORD dwOptions);
        
        virtual HRESULT STDMETHODCALLTYPE Terminate( 
            /* [in] */ DWORD dwOptions);
        
        virtual HRESULT STDMETHODCALLTYPE Suspend(void);
        
        virtual HRESULT STDMETHODCALLTYPE Resume(void);

        virtual HRESULT STDMETHODCALLTYPE Read( 
            /* [length_is][size_is][out][in] */ void __RPC_FAR *pv,
            /* [in] */ ULONG cb,
            /* [out] */ ULONG __RPC_FAR *pcbRead);
        
        virtual HRESULT STDMETHODCALLTYPE Seek( 
            /* [in] */ LARGE_INTEGER dlibMove,
            /* [in] */ DWORD dwOrigin,
            /* [out] */ ULARGE_INTEGER __RPC_FAR *plibNewPosition);
        
        virtual HRESULT STDMETHODCALLTYPE LockRequest( 
            /* [in] */ DWORD dwOptions);
        
        virtual HRESULT STDMETHODCALLTYPE UnlockRequest( void);
		friend class PluginProtocolThread;
};

CLSID PROTOCOL_CLSID = { 0xdc186801, 0x657f, 0x11d4, 
						  {	0xb0, 0xb5,  0x0,  0x50,  0xba,  0xbf,  0xc9,  0x4 	}
						};

IInternetSession *PluginProtocol::s_session			= NULL;
IClassFactory *PluginProtocol::s_cf					= NULL;
PluginProtocolThread *PluginProtocol::s_thread		= NULL;
list<PluginProtocol*> *PluginProtocol::s_protocols	= NULL;
QMutex *PluginProtocol::s_mutex						= NULL;
QWaitCondition	*PluginProtocol::s_condition		= NULL;

PluginProtocol::PluginProtocol()
{
	m_pOIProtSink = NULL;
	m_pOIBindInfo = NULL;
	m_data		  = NULL;
}

PluginProtocol::~PluginProtocol()
{
	Close();
}

class MutexLocker
{
	COPY_RESTRICTED(MutexLocker);
public:
	MutexLocker(QMutex &_mutex) : mutex(_mutex) { mutex.lock(); }
	~MutexLocker() { mutex.unlock(); }
protected:
	QMutex &mutex;
};

void PluginProtocol::Close()
{
	if (m_pOIProtSink){
		m_pOIProtSink->Release();
		m_pOIProtSink = NULL;
	}
	if (m_pOIBindInfo){
		m_pOIBindInfo->Release();
		m_pOIBindInfo = NULL;
	}
	MutexLocker locker(*s_mutex);
	for (list<PluginProtocol*>::iterator it = s_protocols->begin(); it != s_protocols->end(); ++it){
		if ((*it) == this){
			s_protocols->erase(it);
			Release();
			break;
		}
	}
	if (m_data){
		delete m_data;
		m_data = NULL;
	}
}

STDMETHODIMP PluginProtocol::QueryInterface(REFIID riid, LPVOID *ppv)
{
	*ppv = NULL;
	if(IsEqualIID(riid,IID_IUnknown) || IsEqualIID(riid,__uuidof(IInternetProtocol)))
	{
		*ppv = (IInternetProtocol*) this;
		_AddRef();
		return S_OK;
	}
	return E_NOINTERFACE;
}

HRESULT STDMETHODCALLTYPE PluginProtocol::Start(
            /* [in] */ LPCWSTR szUrl,
            /* [in] */ IInternetProtocolSink __RPC_FAR *pOIProtSink,
            /* [in] */ IInternetBindInfo __RPC_FAR *pOIBindInfo,
            /* [in] */ DWORD,
            /* [in] */ DWORD)
{
	m_pOIProtSink = pOIProtSink;
	m_pOIBindInfo = pOIBindInfo;
	m_pOIProtSink->AddRef();
	m_pOIBindInfo->AddRef();
	QString url = CnvBSTR(szUrl);
	int n = url.find(':');
	if (n < 0)
		return INET_E_USE_DEFAULT_PROTOCOLHANDLER;
	QString proto = url.left(n);
	if (proto != "icon")
		return INET_E_USE_DEFAULT_PROTOCOLHANDLER;
	url = url.mid(n + 1);
	n = url.find('?');
	QColor bg = Qt::white;
	QString options;
	bool bBig = false;
	if (n >= 0){
		options = url.mid(n + 1);
		url = url.left(n);
		while (options.length()){
			n = options.find('&');
			QString opt;
			if (n >= 0){
				opt = options.left(n);
				options = options.mid(n + 1);
			}else{
				opt = options;
				options = "";
			}
			if (opt == "big")
				bBig = true;
		}
	}
	n = url.find('.');
	if (n >= 0)
		url = url.left(n);
	const QIconSet *icon = Icon(url.latin1());
	if (icon == NULL)
		return INET_E_USE_DEFAULT_PROTOCOLHANDLER;
	QPixmap pict = icon->pixmap(bBig ? QIconSet::Large : QIconSet::Small, QIconSet::Normal);
	QImage img = pict.convertToImage();
	if (m_data)
		delete m_data;
	m_data = new QBuffer;
	m_data->open(IO_WriteOnly);
	m_data_pos = 0;
	{
		QDataStream out(m_data);
		out.setVersion(1);
		out << img;
	}
	m_data->close();
	MutexLocker lock(*s_mutex);
	AddRef();
	s_protocols->push_back(this);
	s_condition->wakeOne();
	return E_PENDING;
}
 
void PluginProtocol::Register()
{
	CoInternetGetSession(0, &s_session, 0);
	if (s_session){
		s_cf = new CClassFactory<PluginProtocol>;
		HRESULT res = s_session->RegisterNameSpace(s_cf, PROTOCOL_CLSID, L"icon", 0, NULL, 0);
		if (res != S_OK)
			log(L_WARN, "Can't register namespace");
		s_protocols = new list<PluginProtocol*>;
		s_mutex = new QMutex;
		s_condition = new QWaitCondition;
		s_thread = new PluginProtocolThread;
		s_thread->start();
	}
}

void PluginProtocol::Unregister()
{
	if (s_session && s_cf)
		s_session->UnregisterNameSpace(s_cf, L"icon");
	if (s_cf){
		s_cf->Release();
		s_cf = NULL;
	}
	s_condition->wakeAll();
	s_thread->wait();
	delete s_thread;
	delete s_condition;
	delete s_mutex;
	delete s_protocols;
}

void PluginProtocolThread::run()
{
	for (;;){
		MutexLocker lock(*PluginProtocol::s_mutex);
		PluginProtocol::s_condition->wait(PluginProtocol::s_mutex);
		while (!PluginProtocol::s_protocols->empty()){
			PluginProtocol *protocol = PluginProtocol::s_protocols->front();
			protocol->Process();
			protocol->Release();
			PluginProtocol::s_protocols->erase(PluginProtocol::s_protocols->begin());
		}
		if (PluginProtocol::s_cf == NULL)
			break;
	}
}

void PluginProtocol::Process()
{
	SetState(BINDSTATUS_MIMETYPEAVAILABLE, L"image/bmp");
	SetData(BSCF_AVAILABLEDATASIZEUNKNOWN);
	SetResult(S_OK);
}

void PluginProtocol::SetResult(HRESULT result)
{
	PROTOCOLDATA pData;
	pData.grfFlags = 0;
	pData.dwState  = result;
	pData.cbData   = 0;
	pData.pData    = 0;
	m_pOIProtSink->Switch(&pData);
}

void PluginProtocol::SetState(DWORD state, wchar_t *data)
{
	PROTOCOLDATA pData;
	pData.grfFlags = 1;
	pData.dwState  = state;
	pData.cbData   = data ? wcslen(data) : 0;
	pData.pData    = data;
	m_pOIProtSink->Switch(&pData);
}

void PluginProtocol::SetData(DWORD state)
{
	PROTOCOLDATA pData;
	pData.grfFlags = 2;
	pData.dwState  = state;
	pData.cbData   = 0;
	pData.pData    = 0;
	m_pOIProtSink->Switch(&pData);
}
       
HRESULT STDMETHODCALLTYPE PluginProtocol::Continue( 
            /* [in] */ PROTOCOLDATA __RPC_FAR *pProtocolData)
{
	if (pProtocolData->grfFlags == 0)
		return pProtocolData->dwState;
	if (pProtocolData->grfFlags == 1){
		if (m_pOIProtSink == NULL)
			return S_FALSE;
		wchar_t *info = (wchar_t*)(pProtocolData->pData);
		m_pOIProtSink->ReportProgress(pProtocolData->dwState, info ? info : L"");
		return E_PENDING;
	}
	if (pProtocolData->grfFlags == 2){
		if (m_pOIProtSink == NULL)
			return S_FALSE;
		m_pOIProtSink->ReportData(pProtocolData->dwState, 0, 0);
		return E_PENDING;
	}
	return S_FALSE;
}
        
HRESULT STDMETHODCALLTYPE PluginProtocol::Abort( 
            /* [in] */ HRESULT,
            /* [in] */ DWORD)
{
	Close();
	return S_OK;
}
        
HRESULT STDMETHODCALLTYPE PluginProtocol::Terminate( 
            /* [in] */ DWORD)
{
	Close();
	Release();
	return S_OK;
}
        
HRESULT STDMETHODCALLTYPE PluginProtocol::Suspend()
{
	return S_FALSE;
}
        
HRESULT STDMETHODCALLTYPE PluginProtocol::Resume()
{
	return S_FALSE;
}

HRESULT STDMETHODCALLTYPE PluginProtocol::Read( 
            /* [length_is][size_is][out][in] */ void __RPC_FAR *pv,
            /* [in] */ ULONG cb,
            /* [out] */ ULONG __RPC_FAR *pcbRead)
{
	unsigned size = m_data->buffer().size() - m_data_pos;
	if (size < cb)
		cb = size;
	*pcbRead = cb;
	if (cb == 0){
		if (m_pOIProtSink)
			m_pOIProtSink->ReportResult(INET_E_DEFAULT_ACTION, NULL, NULL);
		return S_FALSE;
	}
	memcpy((char*)pv, m_data->buffer() + m_data_pos, cb);
	m_data_pos += cb;
	return S_OK;
}
        
HRESULT STDMETHODCALLTYPE PluginProtocol::Seek( 
            /* [in] */ LARGE_INTEGER,
            /* [in] */ DWORD,
            /* [out] */ ULARGE_INTEGER __RPC_FAR*)
{
	return S_FALSE;
}
        
HRESULT STDMETHODCALLTYPE PluginProtocol::LockRequest( 
            /* [in] */ DWORD)
{
	return S_OK;
}
        
HRESULT STDMETHODCALLTYPE PluginProtocol::UnlockRequest( void)
{
	return S_OK;
}

#endif

class MyMimeSourceFactory : public QMimeSourceFactory
{
public:
    MyMimeSourceFactory();
    ~MyMimeSourceFactory();
    virtual const QMimeSource* data(const QString &abs_name) const;
};

MyMimeSourceFactory::MyMimeSourceFactory()
        : QMimeSourceFactory()
{
}

MyMimeSourceFactory::~MyMimeSourceFactory()
{
}

const QMimeSource *MyMimeSourceFactory::data(const QString &abs_name) const
{
    QString name = abs_name;
    if (name.left(5) == "icon:"){
        name = name.mid(5);
        const QIconSet *icons = Icon(name.latin1());
        if (icons){
            QImage img = icons->pixmap(QIconSet::Small, QIconSet::Normal).convertToImage();
            img = img.convertDepth(32);
            ((QMimeSourceFactory*)this)->setImage(abs_name, img);
        }
    }
    return QMimeSourceFactory::data(abs_name);
}

Icons::Icons()
{
    /* This idea came from kapplication.cpp
       I had a similar idea with setting the old defaultFactory in
       the destructor but this won't work :(
       Christian */
#if COMPAT_QT_VERSION >= 0x030000
    QMimeSourceFactory* oldDefaultFactory = QMimeSourceFactory::takeDefaultFactory();
#endif
    QMimeSourceFactory::setDefaultFactory(new MyMimeSourceFactory());
#if COMPAT_QT_VERSION >= 0x030000
    if (oldDefaultFactory)
        QMimeSourceFactory::addFactory( oldDefaultFactory );
#endif
#ifdef USE_KDE
    connect(kapp, SIGNAL(iconChanged(int)), this, SLOT(iconChanged(int)));
    kapp->addKipcEventMask(KIPC::IconChanged);
    addIcon("about_kde", NULL, "about_kde", 0);
#endif
#ifdef USE_IE
	PluginProtocol::Register();
#endif
    addIcon("ICQ", icq, "licq", 0xFF00 | 60);
    addIcon("MSN", msn, NULL, 0x200);
    addIcon("AIM", aim, NULL, 0);
    addIcon("Yahoo!", yahoo, NULL, 0);
    addIcon("Yahoo!_offline", yahoo_offline, NULL, 0);
    addIcon("Yahoo!_away", yahoo_away, NULL, 0);
    addIcon("Yahoo!_na", yahoo_away, NULL, 0);
    addIcon("Yahoo!_dnd", yahoo_dnd, NULL, 0);
    ICON(away)
    ICON(na)
    ICON(dnd)
    ICON(occupied)
    ICON(ffc)
    KICON(exit)
    KICON(button_ok)
    KICON(button_cancel)
    KICON(1downarrow)
    KICON(1uparrow)
    KICON(1leftarrow)
    KICON(1rightarrow)
    KICON(configure)
    KICON(fileopen)
    KICON(filesave)
    KICON(remove)
    ICON(error)
    ICON(collapsed)
    ICON(expanded)
    KICON(mail_generic)
    KICON(info)
    KICON(text)
    ICON(phone)
    ICON(fax)
    ICON(cell)
    ICON(pager)
    ICON(nophone)
    KICON(webpress)
    KICON(find)
    ICON(nonim)
    ICON(online_on)
    ICON(online_off)
    ICON(grp_on)
    ICON(grp_off)
    ICON(grp_create)
    ICON(grp_rename)
    ICON(home)
    ICON(work)
    ICON(security)
    KICON(run)
    ICON(network)
    ICON(message)
    ICON(file)
    ICON(sms)
    ICON(url)
    ICON(contacts)
    ICON(auth)
    ICON(empty)
    ICON(translit)
    ICON(bgcolor)
    ICON(fgcolor)
    ICON(pict)
    KICON(text_bold)
    KICON(text_italic)
    KICON(text_under)
    KICON(text_strike)
    KICON(cancel)
    KICON(filter)
    KICON(history)
    KICON(editcut)
    KICON(editcopy)
    KICON(editpaste)
    KICON(undo)
    KICON(redo)
    ICON(typing)
    KICON(fileclose)
    KICON(encrypted)
    ICON(smile0)
    ICON(smile1)
    ICON(smile2)
    ICON(smile3)
    ICON(smile4)
    ICON(smile5)
    ICON(smile6)
    ICON(smile7)
    ICON(smile8)
    ICON(smile9)
    ICON(smileA)
    ICON(smileB)
    ICON(smileC)
    ICON(smileD)
    ICON(smileE)
    ICON(smileF)
    ICON(listmsg)
    ICON(urgentmsg)
    KICON(help)
    ICON(btn_more);
    ICON(chat);
    addIcon("encoding", encoding, "charset", 0);
    addIcon("add", add, "bookmark_add", 0);
    addIcon("new", p_new, "filenew", 0);
}

Icons::~Icons()
{
#if COMPAT_QT_VERSION < 0x030000
    QMimeSourceFactory::setDefaultFactory(new QMimeSourceFactory());
#endif
#ifdef USE_IE
	PluginProtocol::Unregister();
#endif
}

void *Icons::processEvent(Event *e)
{
    IconDef *def;
    switch (e->type()){
    case EventAddIcon:
        def = (IconDef*)(e->param());
        addIcon(def->name, def->xpm, def->system, def->flags);
        return e->param();
    case EventGetIcon:
        return (void*)getIcon((const char*)(e->param()));
    case EventIconChanged:{
            remove("online");
            remove("offline");
            remove("inactive");
            for (PIXMAP_MAP::iterator it = icons.begin(); it != icons.end();){
                if ((*it).second.xpm){
                    ++it;
                    continue;
                }
                icons.erase(it);
                it = icons.begin();
            }
            break;
        }
    case EventGetIcons:
        fill((list<string>*)(e->param()));
        return e->param();
    default:
        break;
    }
    return NULL;
}

void Icons::remove(const char *name)
{
    PIXMAP_MAP::iterator it = icons.find(name);
    if (it != icons.end())
        icons.erase(it);
    it = bigIcons.find(name);
    if (it != bigIcons.end())
        bigIcons.erase(it);
}

static QPixmap makeOffline(unsigned flags, const QPixmap &p)
{
    unsigned swapColor = flags & ICON_COLOR_MASK;
    QImage image = p.convertToImage();
    unsigned int *data = (image.depth() > 8) ? (unsigned int *)image.bits() :
                         (unsigned int *)image.colorTable();
    int pixels = (image.depth() > 8) ? image.width()*image.height() :
                 image.numColors();
    for (int i = 0; i < pixels; i++){
        QColor c(qRed(data[i]), qGreen(data[i]), qBlue(data[i]));
        int a = qAlpha(data[i]);
        int h, s, v;
        c.hsv(&h, &s, &v);
        if (swapColor){
            h = (swapColor * 2 - h) & 0xFF;
            c.setHsv(h, s, v);
        }else{
            c.setHsv(h, 0, v * 3 / 4);
        }
        data[i] = qRgba(c.red(), c.green(), c.blue(), a);
    }
    QPixmap pict;
    pict.convertFromImage(image);
    return pict;
}

static QIconSet makeOffline(unsigned flags, const QIconSet *icon)
{
    QPixmap pict1 = icon->pixmap(QIconSet::Small, QIconSet::Normal);
    QPixmap pict2 = icon->pixmap(QIconSet::Large, QIconSet::Normal);
    pict1 = makeOffline(flags, pict1);
    pict2 = makeOffline(flags, pict2);
    return QIconSet(pict1, pict2);
}

static QPixmap makeInactive(const QPixmap &p)
{
    QImage image = p.convertToImage();
    unsigned int *data = (image.depth() > 8) ? (unsigned int *)image.bits() :
                         (unsigned int *)image.colorTable();
    int pixels = (image.depth() > 8) ? image.width()*image.height() :
                 image.numColors();
    for (int i = 0; i < pixels; i++){
        QColor c(qRed(data[i]), qGreen(data[i]), qBlue(data[i]));
        int a = qAlpha(data[i]);
        int h, s, v;
        c.hsv(&h, &s, &v);
        c.setHsv(h, s / 8, v);
        data[i] = qRgba(c.red(), c.green(), c.blue(), a);
    }
    QPixmap pict;
    pict.convertFromImage(image);
    return pict;
}

static QIconSet makeInactive(const QIconSet *icon)
{
    QPixmap pict1 = icon->pixmap(QIconSet::Small, QIconSet::Normal);
    QPixmap pict2 = icon->pixmap(QIconSet::Large, QIconSet::Normal);
    pict1 = makeInactive(pict1);
    pict2 = makeInactive(pict2);
    return QIconSet(pict1, pict2);
}

static QPixmap makeInvisible(unsigned flags, const QPixmap &p)
{
    unsigned swapColor = flags & ICON_COLOR_MASK;
    char shift = (flags >> 8) & 0xFF;
    QImage image = p.convertToImage();
    if (image.depth() != 32)
        image = image.convertDepth(32);
    unsigned int *data = (unsigned int*)image.bits();
    for (int y = 0; y < image.width(); y++){
        int x = image.width() / 2 - (y - image.height() / 2) * 2 / 3 + shift;
        if (x < 0)
            x = 0;
        if (x > image.width())
            x = image.width();
        unsigned int *line = data + y * (image.width()) + x;
        for (; x < image.width(); x++, line++){
            QColor c(qRed(*line), qGreen(*line), qBlue(*line));
            int a = qAlpha(*line);
            int h, s, v;
            c.hsv(&h, &s, &v);
            if (swapColor){
                h = (swapColor * 2 - h) & 0xFF;
                c.setHsv(h, s / 2, v * 3 / 4);
            }else{
                c.setHsv(h, s / 2, v * 3 / 4);
            }
            *line = qRgba(c.red(), c.green(), c.blue(), a);
        }
    }
    QPixmap pict;
    pict.convertFromImage(image);
    return pict;
}

static QIconSet makeInvisible(unsigned flags, const QIconSet *icon)
{
    QPixmap pict1 = icon->pixmap(QIconSet::Small, QIconSet::Normal);
    QPixmap pict2 = icon->pixmap(QIconSet::Large, QIconSet::Normal);
    pict1 = makeInvisible(flags, pict1);
    pict2 = makeInvisible(flags, pict2);
    return QIconSet(pict1, pict2);
}

static QPixmap merge(const QPixmap &p1, const QPixmap &p2)
{
    QImage img1 = p1.convertToImage();
    if (img1.depth() != 32)
        img1 = img1.convertDepth(32);
    QImage img2 = p2.convertToImage();
    if (img2.depth() != 32)
        img2 = img2.convertDepth(32);
    unsigned int *data1 = (unsigned int*)img1.bits();
    unsigned int *data2 = (unsigned int*)img2.bits();
    for (int y1 = 0; y1 < img1.height(); y1++){
        int y2 = y1 - (img1.height() - img2.height());
        if ((y2 < 0) || (y2 >= img2.height()))
            continue;
        unsigned int *line1 = data1 + y1 * img1.width();
        unsigned int *line2 = data2 + y2 * img2.width();
        int w = img1.width();
        if (w < img2.width()){
            line2 += (img2.width() - w) / 2;
        }else if (w > img2.width()){
            w = img2.width();
            line1 += (img1.width() - w) / 2;
        }
        for (int i = 0; i < w; i++, line1++, line2++){
            int r1 = qRed(*line1);
            int g1 = qGreen(*line1);
            int b1 = qBlue(*line1);
            int a1 = qAlpha(*line1);
            int r2 = qRed(*line2);
            int g2 = qGreen(*line2);
            int b2 = qBlue(*line2);
            int a2 = qAlpha(*line2);
            r1 = (r2 * a2 + r1 * (255 - a2)) / 255;
            g1 = (g2 * a2 + g1 * (255 - a2)) / 255;
            b1 = (b2 * a2 + b1 * (255 - a2)) / 255;
            if (a2 > a1)
                a1 = a2;
            *line1 = qRgba(r1, g1, b1, a1);
        }
    }
    QPixmap res;
    res.convertFromImage(img1);
    return res;
}


static QIconSet mergeIcon(const QIconSet *icon, const QPixmap &pict)
{
    QPixmap pict1 = icon->pixmap(QIconSet::Small, QIconSet::Normal);
    QPixmap pict2 = icon->pixmap(QIconSet::Large, QIconSet::Normal);
    pict1 = merge(pict1, pict);
    pict2 = merge(pict2, pict);
    return QIconSet(pict1, pict2);
}

QPixmap loadPict(const char *name)
{
    QPixmap p;
    string fname = "pict/";
    fname += name;
    fname += ".png";
    fname = user_file(fname.c_str());
    QFile f(QFile::decodeName(fname.c_str()));
    if (f.exists()){
        p = QPixmap(f.name());
        if (!p.isNull())
            return p;
    }
    fname = "pict/";
    fname += name;
    fname += ".png";
    fname = app_file(fname.c_str());
    f.setName(QFile::decodeName(fname.c_str()));
    if (f.exists())
        p = QPixmap(f.name());
    return p;
}

const QIconSet *Icons::getIcon(const char *name)
{
    if (name == NULL)
        return NULL;
    PIXMAP_MAP::iterator it = icons.find(name);
    if (it == icons.end()){
        string n = name;
        if (n == "online"){
            unsigned i;
            const QIconSet *icon = NULL;
            const char *icon_name = NULL;
            for (i = 0; i < getContacts()->nClients(); i++){
                Client *client = getContacts()->getClient(i);
                icon_name = client->protocol()->description()->icon;
                icon = Icon(icon_name);
                if (icon)
                    break;
            }
            if (icon == NULL){
                icon_name = "ICQ";
                icon = Icon(icon_name);
            }
            if (icon == NULL)
                return NULL;
            unsigned flags = 0;
            it = icons.find(icon_name);
            if (it != icons.end())
                flags = (*it).second.flags;
            PictDef *d = addIcon(name, NULL, NULL, flags);
            d->iconSet = *icon;
            return icon;
        }
        if (n == "offline"){
            const QIconSet *icon = Icon("online");
            if (icon == NULL)
                return NULL;
            unsigned flags = 0;
            it = icons.find("online");
            if (it != icons.end())
                flags = (*it).second.flags;
            QIconSet offline = makeOffline(flags, icon);
            PictDef *d = addIcon(name, NULL, NULL, flags);
            d->iconSet = offline;
            return &d->iconSet;
        }
        if (n == "inactive"){
            const QIconSet *icon = Icon("online");
            if (icon == NULL)
                return NULL;
            unsigned flags = 0;
            it = icons.find("online");
            if (it != icons.end())
                flags = (*it).second.flags;
            QIconSet offline = makeInactive(icon);
            PictDef *d = addIcon(name, NULL, NULL, flags);
            d->iconSet = offline;
            return &d->iconSet;
        }
        if (n == "invisible"){
            const QIconSet *icon = Icon("online");
            if (icon == NULL)
                return NULL;
            unsigned flags = 0;
            it = icons.find("online");
            if (it != icons.end())
                flags = (*it).second.flags;
            QIconSet invisible = makeInvisible(flags, icon);
            PictDef *d = addIcon(name, NULL, NULL, flags);
            d->iconSet = invisible;
            return &d->iconSet;
        }
        int pos = n.find('_');
        if (pos > 0){
            const QIconSet *icon = Icon(n.substr(0, pos).c_str());
            unsigned flags = 0;
            it = icons.find(n.substr(0, pos).c_str());
            if (it != icons.end())
                flags = (*it).second.flags;
            QIconSet res;
            if (icon){
                string s = n.substr(pos + 1);
                if (s == "online"){
                    res = *icon;
                }else if (s == "offline"){
                    res = makeOffline(flags, icon);
                }else if (s == "invisible"){
                    res = makeInvisible(flags, icon);
                }else if (s == "inactive"){
                    res = makeInactive(icon);
                }else{
                    const QIconSet *i = Icon(s.c_str());
                    if (i){
                        res = mergeIcon(icon, i->pixmap(QIconSet::Small, QIconSet::Normal));
                    }else{
                        it = icons.find(s.c_str());
                        if (it != icons.end()){
                            flags = (*it).second.flags;
                            res = makeOffline(flags, icon);
                        }
                    }
                }
            }
            if (res.pixmap(QIconSet::Small, QIconSet::Normal).width()){
                PictDef *d = addIcon(name, NULL, NULL, flags);
                d->iconSet = res;
                return &d->iconSet;
            }
        }
        log(L_DEBUG, "Icon %s not found", name);
        return NULL;
    }
    PictDef &p = (*it).second;
    if (p.iconSet.pixmap(QIconSet::Small, QIconSet::Normal).width())
        return &p.iconSet;
#ifdef USE_KDE
    if (p.system){
        QPixmap pict = SmallIconSet(p.system).pixmap(QIconSet::Small, QIconSet::Normal);
        if (!pict.isNull()){
            QPixmap pict1 = DesktopIconSet(p.system).pixmap(QIconSet::Large, QIconSet::Normal);
            if (pict.isNull()){
                p.iconSet = QIconSet(pict);
            }else{
                p.iconSet = QIconSet(pict, pict1);
            }
            return &p.iconSet;
        }
    }
#endif
    QPixmap pict = loadPict(name);
    if (!pict.isNull()){
        if (pict.width() > 16){
            QImage img = pict.convertToImage();
            img = img.smoothScale(16, 16);
            QPixmap pict1(16, 16);
            pict1.convertFromImage(img);
            p.iconSet = QIconSet(pict1, pict);
        }else{
            p.iconSet = QIconSet(pict);
        }
        return &p.iconSet;
    }
    if (p.xpm){
        p.iconSet = QIconSet(QPixmap(p.xpm));
        return &p.iconSet;
    }
    log(L_DEBUG, "Icon %s is empty", name);
    return NULL;
}

PictDef *Icons::addIcon(const char *name, const char **xpm, const char *system, unsigned flags)
{
    PictDef *res;
    PIXMAP_MAP::iterator it = icons.find(name);
    if (it == icons.end()){
        PictDef p;
        icons.insert(PIXMAP_MAP::value_type(name, p));
        it = icons.find(name);
    }
    res = &(*it).second;
    res->iconSet = QIconSet();
    res->xpm     = xpm;
    res->system  = system;
    res->flags	 = flags;
    return res;
}

void Icons::iconChanged(int)
{
    Event e(EventIconChanged, NULL);
    e.process();
}

void Icons::fill(list<string> *names)
{
    for (PIXMAP_MAP::iterator it = icons.begin(); it != icons.end(); ++it)
        names->push_back((*it).first.c_str());
}

#ifndef WIN32
#include "icons.moc"
#endif


