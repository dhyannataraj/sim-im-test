// SimControl.cpp : Implementation of CSimControl
#include <atlbase.h>
//You may derive a class from CComModule and use it if you want to override
//something, but do not change the name of _Module
extern CComModule _Module;
#include <atlcom.h>
#include "Simremote.h"
#include "SimControl.h"

/////////////////////////////////////////////////////////////////////////////
// CSimControl


STDMETHODIMP CSimControl::get_Running(BOOL *pVal)
{
    // TODO: Add your implementation code here

    return S_OK;
}

STDMETHODIMP CSimControl::Process(BSTR in_str, BSTR *out_str)
{
    // TODO: Add your implementation code here

    return S_OK;
}
