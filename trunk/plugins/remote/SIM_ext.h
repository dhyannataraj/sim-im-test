// SIM_ext.h : Declaration of the CSIM_ext

#ifndef __SIM_EXT_H_
#define __SIM_EXT_H_

#include "resource.h"       // main symbols
#include "comdef.h"
#include "shlobj.h"
#include "IShellExtInitImpl.h"			// IShellExtInit
#include "IContextMenuImpl.h"			// IContextMenu

#if _MSC_VER > 1020
#include <yvals.h>              
#pragma warning(disable: 4097)
#pragma warning(disable: 4244)  
#pragma warning(disable: 4275)
#pragma warning(disable: 4514)
#pragma warning(disable: 4710)  
#pragma warning(disable: 4786)  
#pragma warning(push)
#pragma warning(disable: 4018)  
#pragma warning(disable: 4100)  
#pragma warning(disable: 4146)  
#pragma warning(disable: 4511)  
#pragma warning(disable: 4512)  
#pragma warning(disable: 4530)  
#pragma warning(disable: 4663)  
#endif

#include <string>
#include <list>
#include <map>

using namespace std;

typedef struct ItemInfo
{
    string		text;
    HICON		icon;
    unsigned	id;
} ItemInfo;

typedef map<string, HICON>		ICON_MAP;
typedef map<unsigned, ItemInfo>	ITEM_MAP;

/////////////////////////////////////////////////////////////////////////////
// CSIM_ext
class ATL_NO_VTABLE CSIM_ext :
            public CComObjectRootEx<CComSingleThreadModel>,
            public CComCoClass<CSIM_ext, &CLSID_SIM_ext>,
            public ISIM_ext,
            public IObjectWithSiteImpl<CSIM_ext>,
            public IShellExtInitImpl,
            public IContextMenuImpl
{
public:
    CSIM_ext();
    ~CSIM_ext();
    LPDATAOBJECT lpData;
    static HINSTANCE hInstance;

    DECLARE_REGISTRY_RESOURCEID(IDR_SIM_EXT)
    DECLARE_NOT_AGGREGATABLE(CSIM_ext)

    DECLARE_PROTECT_FINAL_CONSTRUCT()

    BEGIN_COM_MAP(CSIM_ext)
    COM_INTERFACE_ENTRY(ISIM_ext)
    COM_INTERFACE_ENTRY(IShellExtInit)
    COM_INTERFACE_ENTRY(IContextMenu)
    COM_INTERFACE_ENTRY(IContextMenu2)
    COM_INTERFACE_ENTRY(IContextMenu3)
    COM_INTERFACE_ENTRY(IObjectWithSite)
    END_COM_MAP()

    ICON_MAP	  m_icons;
    ITEM_MAP	  m_items;
    ItemInfo	  getItemInfo(unsigned id);
    HICON		  createIcon(const char *name);
    HBITMAP		  createBitmap(string &info);

    // ISIM_ext
public:
    STDMETHOD(QueryContextMenu)(THIS_
                                HMENU hmenu,
                                UINT indexMenu,
                                UINT idCmdFirst,
                                UINT idCmdLast,
                                UINT uFlags);

    STDMETHOD(InvokeCommand)(THIS_
                             LPCMINVOKECOMMANDINFO lpici);

    STDMETHOD(HandleMenuMsg)(UINT, WPARAM, LPARAM);

    STDMETHOD(HandleMenuMsg2)(UINT, WPARAM, LPARAM, LRESULT *);

    STDMETHOD(GetCommandString)(THIS_
                                UINT        idCmd,
                                UINT        uType,
                                UINT      * pwReserved,
                                LPSTR       pszName,
                                UINT        cchMax);

    STDMETHOD(Initialize)(THIS_ LPCITEMIDLIST pidlFolder,
                          LPDATAOBJECT lpdobj, HKEY hkeyProgID);
private:
    void MeasureItem(LPMEASUREITEMSTRUCT);
    void DrawMenuItem(LPDRAWITEMSTRUCT);
};

#endif //__SIM_EXT_H_
