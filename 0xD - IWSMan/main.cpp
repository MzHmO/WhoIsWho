#include <windows.h>
#include <iostream>
#include <locale.h>


void WhoIsWho();

int main()
{
    setlocale(LC_ALL, "");
    WhoIsWho();
    return 0;
}

#include <wsmandisp.h>
void WhoIsWho() {
    IWSManSession* pWsSess = NULL;
    IWSManConnectionOptions* pOptions = NULL;
    IWSMan* pLoc = NULL;
    HRESULT hr;

    hr = CoInitializeEx(0, COINIT_MULTITHREADED);
    
    if (FAILED(hr))
    {
        return;
    }
    const IID IID_IWSMan = { 0x190D8637, 0x5CD3, 0x496D, {0xAD, 0x24, 0x69, 0x63, 0x6B, 0xB5, 0xA3, 0xB5 } };
    const CLSID CLSID_WSMan = { 0xBCED617B, 0xEC03, 0x420b, {0x85, 0x08, 0x97, 0x7D, 0xC7, 0xA6, 0x86, 0xBD} };
    hr = CoCreateInstance(CLSID_WSMan, 0, CLSCTX_INPROC_SERVER, IID_IWSMan, (LPVOID*)&pLoc);

    if (FAILED(hr))
    {
        return;
    }

    BSTR commandLine = (BSTR)malloc(MAX_PATH);
    pLoc->get_CommandLine(&commandLine);
    
    
    std::wcout << commandLine << std::endl;
    
    return;
}