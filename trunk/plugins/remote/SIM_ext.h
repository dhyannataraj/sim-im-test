// SIM_ext.h : Declaration of the CSIM_ext

#ifndef __SIM_EXT_H_
#define __SIM_EXT_H_

#include "resource.h"       // main symbols
#include "comdef.h"
#include "shlobj.h"

/////////////////////////////////////////////////////////////////////////////
// CSIM_ext
class ATL_NO_VTABLE CSIM_ext :
            public CComObjectRootEx<CComSingleThreadModel>,
            public CComCoClass<CSIM_ext, &CLSID_SIM_ext>,
            public ISIM_ext,
            public IContextMenu,
            public IShellExtInit
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
    COM_INTERFACE_ENTRY(IContextMenu)
    COM_INTERFACE_ENTRY(IShellExtInit)
    END_COM_MAP()

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

    STDMETHOD(GetCommandString)(THIS_
                                UINT        idCmd,
                                UINT        uType,
                                UINT      * pwReserved,
                                LPSTR       pszName,
                                UINT        cchMax);

    STDMETHOD(Initialize)(THIS_ LPCITEMIDLIST pidlFolder,
                          LPDATAOBJECT lpdobj, HKEY hkeyProgID);
};

#endif //__SIM_EXT_H_
