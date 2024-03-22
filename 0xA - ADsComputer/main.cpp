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

#include <iads.h>
#include <AdsHlp.h>
#pragma comment(lib, "Activeds.lib")
void WhoIsWho()
{
    HRESULT hr;

    hr = CoInitialize(NULL);

    IADsComputer* pComp = NULL;
    WCHAR computerName[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD computerNameLen = sizeof(computerName) / sizeof(computerName[0]);

    if (!GetComputerNameW(computerName, &computerNameLen)) {
        return;
    }
    size_t adspathLen = wcslen(L"WinNT://") + wcslen(computerName) + wcslen(L",computer") + 1;
    LPWSTR adspath = new WCHAR[adspathLen];
    swprintf(adspath, adspathLen, L"WinNT://%s,computer", computerName);

    BSTR bstr = NULL;

    const IID IID_IADsComputer = { 0xEFE3CC70, 0x1D9F, 0x11CF, { 0xB1, 0xF3, 0x02, 0x60, 0x8C, 0x9E, 0x75, 0x53 } };
    hr = ADsGetObject(adspath, IID_IADsComputer, (void**)&pComp);
    if (FAILED(hr)) { goto Cleanup; }

    hr = pComp->get_Owner(&bstr);
    if (FAILED(hr)) { goto Cleanup; }

    std::wcout << bstr << std::endl;

Cleanup:
    if (pComp) pComp->Release();
    if (bstr) SysFreeString(bstr);
    delete[] adspath;
    CoUninitialize();
    return;
}