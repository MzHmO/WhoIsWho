#include <windows.h>
#include <locale>
#include <iostream>

void WhoIsWho();

int main()
{
    setlocale(LC_ALL, "");
    WhoIsWho();
    return 0;
}

#include <comdef.h>
#include <Wbemidl.h>
#pragma comment(lib, "wbemuuid.lib")
void WhoIsWho()
{
    HRESULT hres;

    hres = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hres))
    {
        return;               
    }

    hres = CoInitializeSecurity(
        NULL,
        -1,                       
        NULL,                     
        NULL,                     
        RPC_C_AUTHN_LEVEL_DEFAULT,   
        RPC_C_IMP_LEVEL_IMPERSONATE, 
        NULL,                       
        EOAC_NONE,                 
        NULL                       
    );

    IWbemLocator* pLoc = NULL;

    hres = CoCreateInstance(
        CLSID_WbemLocator,
        0,
        CLSCTX_INPROC_SERVER,
        IID_IWbemLocator, (LPVOID*)&pLoc);

    if (FAILED(hres))
    {
        std::cout << "Failed to create IWbemLocator object. Err code = 0x"
            << std::hex << hres << std::endl;
        CoUninitialize();
        return;              
    }

    IWbemServices* pSvc = NULL;

    hres = pLoc->ConnectServer(
        _bstr_t(L"ROOT\\CIMV2"),
        NULL,                   
        NULL,               
        0,                     
        NULL,                 
        0,
        0,                      
        &pSvc                   
    );

    if (FAILED(hres))
    {
        pLoc->Release();
        CoUninitialize();
        return;               
    }

    hres = CoSetProxyBlanket(
        pSvc,                        
        RPC_C_AUTHN_WINNT,         
        RPC_C_AUTHZ_NONE,            
        NULL,                      
        RPC_C_AUTHN_LEVEL_CALL,     
        RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL,                        
        EOAC_NONE                   
    );

    if (FAILED(hres))
    {
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return;             
    }

    IEnumWbemClassObject* pEnumerator = NULL;
    hres = pSvc->ExecQuery(
        bstr_t("WQL"),
        bstr_t("SELECT UserName FROM Win32_ComputerSystem"),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        NULL,
        &pEnumerator);

    if (FAILED(hres))
    {
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return;      
    }

    IWbemClassObject* pclsObj = NULL;
    ULONG uReturn = 0;

    while (pEnumerator)
    {
        HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1,
            &pclsObj, &uReturn);

        if (0 == uReturn)
        {
            break;
        }

        VARIANT vtProp;

        hr = pclsObj->Get(L"UserName", 0, &vtProp, 0, 0);
        std::wcout << vtProp.bstrVal << std::endl;
        VariantClear(&vtProp);

        pclsObj->Release();
    }

    pSvc->Release();
    pLoc->Release();
    pEnumerator->Release();
    CoUninitialize();

    return;
}