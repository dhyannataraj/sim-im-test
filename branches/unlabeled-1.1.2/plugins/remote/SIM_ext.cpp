// SIM_ext.cpp : Implementation of CSIM_ext
#include <atlbase.h>
extern CComModule _Module;
#include <atlcom.h>
#include "Simext.h"
#include "SIM_ext.h"
#include <stdio.h>

static bool (*ProcessStr)(BSTR in_str, BSTR *out_str) = NULL;

HINSTANCE CSIM_ext::hInstance;

/////////////////////////////////////////////////////////////////////////////
// CSIM_ext

CSIM_ext::CSIM_ext()
{
    lpData = NULL;
    if (ProcessStr == NULL){
        char name[512];
        GetModuleFileName(hInstance, name, sizeof(name));
        char *r = strrchr(name, '\\');
        if (r){
            r++;
        }else{
            r = name;
        }
        strcpy(r, "simremote.dll");
        HINSTANCE hLib = LoadLibrary(name);
        (DWORD&)ProcessStr = (DWORD)GetProcAddress(hLib, "ProcessStr");
    }
}

CSIM_ext::~CSIM_ext()
{
    if (lpData)
        lpData->Release();
}

HRESULT CSIM_ext::QueryContextMenu(HMENU hmenu,
                                   UINT indexMenu,
                                   UINT idCmdFirst,
                                   UINT idCmdLast,
                                   UINT uFlags)
{
    if (lpData == NULL)
        return 0;
    if (((uFlags & 0x0000000F) == CMF_NORMAL) || (uFlags & CMF_EXPLORE)){
        CComBSTR in("CONTACTS");
        CComBSTR out;
        unsigned n = 0;
        if (ProcessStr && ProcessStr(in, &out)){
            HMENU hsub = NULL;
            size_t size = WideCharToMultiByte(CP_ACP, 0, out, wcslen(out), 0, 0, NULL, NULL);
            char *res = new char[size + 1];
            size = WideCharToMultiByte(CP_ACP, 0, out, wcslen(out), res, size, NULL, NULL);
            res[size] = 0;
            if (res[0] == '>'){
                char *name = res + 1;
                unsigned id = 0;
                for (char *p = name; *p; p++){
                    if ((*p == ' ') && (id == 0)){
                        id = atol(name);
                        name = p + 1;
                    }
                    if (*p == '\n'){
                        *p = 0;
                        if (strlen(name)){
                            if (hsub == NULL)
                                hsub = CreatePopupMenu();
                            AppendMenu(hsub, MF_STRING, idCmdFirst + id, name);
                            if (id > n)
                                n = id;
                            id = 0;
                        }
                        name = p + 1;
                    }
                }
            }
            delete[] res;
            if (hsub != NULL){
                InsertMenu(hmenu, 0, MF_BYPOSITION | MF_POPUP | MF_STRING, (unsigned)hsub, "Send to SIM contact");
            }
        }
        return n;
    }
    return 0;
}

#ifndef CF_HDROP
#define CF_HDROP            15
#endif

HRESULT CSIM_ext::InvokeCommand(LPCMINVOKECOMMANDINFO lpici)
{
    if (lpData == NULL)
        return S_OK;
    STGMEDIUM stgmedium = { TYMED_HGLOBAL, NULL };
    FORMATETC formatetc = { CF_HDROP,
                            NULL,
                            DVASPECT_CONTENT,
                            -1,
                            TYMED_HGLOBAL
                          };
    HRESULT hr = lpData->GetData(&formatetc, &stgmedium);
    if (SUCCEEDED(hr)){
        char *drop_files = (char*)GlobalLock(stgmedium.hGlobal);
        DROPFILES *files = (DROPFILES*)drop_files;
        drop_files += files->pFiles;
        CComBSTR in("SENDFILE \"");
        if (files->fWide){
            in += (unsigned short*)drop_files;
        }else{
            in += drop_files;
        }
        in += "\" ";
        char b[12];
        sprintf(b, "%u", lpici->lpVerb);
        in += b;
        GlobalUnlock(stgmedium.hGlobal);
        CComBSTR out;
        if (ProcessStr)
            ProcessStr(in, &out);
    }
    GlobalFree(stgmedium.hGlobal);
    if (lpData){
        lpData->Release();
        lpData = NULL;
    }
    return S_OK;
}

HRESULT CSIM_ext::GetCommandString(UINT        idCmd,
                                   UINT        uType,
                                   UINT      * pwReserved,
                                   LPSTR       pszName,
                                   UINT        cchMax)
{
    return S_OK;
}

HRESULT CSIM_ext::Initialize(LPCITEMIDLIST pidlFolder, LPDATAOBJECT lpdobj, HKEY hkeyProgID)
{
    if (lpData)
        lpData->Release();
    lpData = lpdobj;
    lpData->AddRef();
    return S_OK;
}

