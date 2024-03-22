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

#include <ActiveDS.h>
void WhoIsWho()
{
    const IID IID_IADsWinNTSystemInfo = { 0x6C6D65DC, 0xAFD1, 0x11D2, { 0x9C, 0xB9, 0x00, 0x00, 0xF8, 0x7A, 0x36, 0x9E } };
    const CLSID CLSID_WinNTSystemInfo = { 0x66182EC4, 0xAFD1, 0x11d2, { 0x9C, 0xB9, 0x00, 0x00, 0xF8, 0x7A, 0x36, 0x9E } };

    HRESULT hr;

    hr = CoInitialize(NULL);

    IADsWinNTSystemInfo* pNtSys;
    hr = CoCreateInstance(CLSID_WinNTSystemInfo,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_IADsWinNTSystemInfo,
        (void**)&pNtSys);

    BSTR bstr;
    hr = pNtSys->get_UserName(&bstr);
    if (SUCCEEDED(hr)) {
        std::wcout << bstr << std::endl;
        SysFreeString(bstr);
    }

    if (pNtSys) {
        pNtSys->Release();
    }

    CoUninitialize();
}