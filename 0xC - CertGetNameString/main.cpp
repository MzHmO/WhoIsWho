#include <windows.h>
#include <iostream>

void WhoIsWho();
HRESULT GetLdapPathFromCurrentDomain(std::wstring& ldapPath);
bool IsUserExistsInAD(const std::wstring& userName);
bool IsUserExists(const std::wstring& userFullName);

int main()
{
    WhoIsWho();
    return 0;
}

#include <lm.h> 
#include <dsgetdc.h>
#include <comdef.h>
#include <activeds.h>

#pragma comment(lib, "crypt32.lib")
#pragma comment(lib, "ActiveDS.lib")
#pragma comment(lib, "Netapi32.lib")
void WhoIsWho() {
    HCERTSTORE hCertStore;
    PCCERT_CONTEXT pCertContext = NULL;

    hCertStore = CertOpenSystemStore(NULL, L"MY");

    if (hCertStore == NULL) {
        return;
    }

    while (pCertContext = CertEnumCertificatesInStore(hCertStore, pCertContext)) {
        DWORD dwData;
        wchar_t szNameHolder[512];
        wchar_t szNameIssuer[512];

        if (!CertGetNameStringW(pCertContext, CERT_NAME_SIMPLE_DISPLAY_TYPE, 0, NULL, szNameHolder, 523)) {
            continue;
        }

        if (!CertGetNameStringW(pCertContext, CERT_NAME_SIMPLE_DISPLAY_TYPE, CERT_NAME_ISSUER_FLAG, NULL, szNameIssuer, 523)) {
            continue;
        }

        if (IsUserExists(szNameHolder) || IsUserExistsInAD(szNameHolder)) {
            if (wcscmp(szNameHolder, szNameIssuer) == 0) {
                std::wcout << szNameHolder << L" [Probably it is yours username]" << std::endl;
            }
            else {
                std::wcout << szNameHolder << L" [Existing system user]" << std::endl;
            }
        }
    }

    if (pCertContext) {
        CertFreeCertificateContext(pCertContext);
    }

    CertCloseStore(hCertStore, 0);
    return;
}

HRESULT GetLdapPathFromCurrentDomain(std::wstring& ldapPath) {
    DOMAIN_CONTROLLER_INFO* info;
    DWORD dwResult = DsGetDcName(NULL, NULL, NULL, NULL, DS_DIRECTORY_SERVICE_REQUIRED, &info);
    if (dwResult == ERROR_SUCCESS) {
        ldapPath = L"LDAP://";
        ldapPath += info->DomainName;
        NetApiBufferFree(info);
        return S_OK;
    }
    return HRESULT_FROM_WIN32(dwResult);
}

bool IsUserExistsInAD(const std::wstring& userName) {
    std::wstring ldapPath;
    HRESULT hr = GetLdapPathFromCurrentDomain(ldapPath);
    if (FAILED(hr)) {
        return false;
    }

    IADsContainer* pContainer = NULL;
    IEnumVARIANT* pEnum = NULL;

    CoInitialize(NULL);

    const IID IID_IADsContainer = { 0x001677D0, 0xFD16, 0x11CE, {0xAB, 0xC4 , 0x02, 0x60, 0x8C, 0x9E, 0x75, 0x53} };
    hr = ADsGetObject(ldapPath.c_str(), IID_IADsContainer, (void**)&pContainer);
    if (FAILED(hr)) {
        CoUninitialize();
        return false;
    }

    hr = ADsBuildEnumerator(pContainer, &pEnum);
    if (FAILED(hr)) {
        pContainer->Release();
        CoUninitialize();
        return false;
    }

    VARIANT var;
    IDispatch* pDisp = NULL;
    IADsUser* pUser = NULL;
    bool userFound = false;

    const IID IID_IADsUser = { 0x3E37E320 , 0x17E2 , 0x11CF, { 0xAB, 0xC4, 0x02, 0x60, 0x8C, 0x9E, 0x75, 0x53 } };
    while (ADsEnumerateNext(pEnum, 1, &var, NULL) == S_OK) {
        pDisp = V_DISPATCH(&var);
        hr = pDisp->QueryInterface(IID_IADsUser, (void**)&pUser);
        if (SUCCEEDED(hr)) {
            BSTR bstrName;
            hr = pUser->get_Name(&bstrName);
            if (SUCCEEDED(hr)) {
                if (_wcsicmp(userName.c_str(), bstrName) == 0) {
                    userFound = true;
                    SysFreeString(bstrName);
                    break;
                }
                SysFreeString(bstrName);
            }
            pUser->Release();
        }
        pDisp->Release();
        VariantClear(&var);
    }

    ADsFreeEnumerator(pEnum);
    pContainer->Release();
    CoUninitialize();

    return userFound;
}

bool IsUserExists(const std::wstring& userFullName) {
    USER_INFO_0* userBuffer;
    DWORD entriesRead;
    DWORD totalEntries;
    bool userExists = false;

    std::wstring userName;
    const wchar_t* slashPos = wcschr(userFullName.c_str(), L'\\');
    if (slashPos) {
        userName = slashPos + 1;
    }
    else {
        userName = userFullName;
    }

    if (NERR_Success == NetUserEnum(NULL, 0, FILTER_NORMAL_ACCOUNT, (LPBYTE*)&userBuffer, MAX_PREFERRED_LENGTH, &entriesRead, &totalEntries, NULL)) {
        for (DWORD i = 0; i < entriesRead; i++) {
            if (_wcsicmp(userName.c_str(), userBuffer[i].usri0_name) == 0) {
                userExists = true;
                break;
            }
        }
        NetApiBufferFree(userBuffer);
    }
    return userExists;
}