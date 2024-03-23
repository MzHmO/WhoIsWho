#include <windows.h>
#include <iostream>

void WhoIsWho();
HRESULT GetLdapPathFromCurrentDomain(std::wstring& ldapPath);
bool IsUserExistsInAD(const std::wstring& userName);
bool IsUserExists(const std::wstring& userFullName);

int main()
{
    /*
    std::wstring usr = L"CRINGE\\petka";
    if (IsUserExistsInAD(usr))
    {
        std::wcout << L"Valid User" << std::endl;
        return 0;
    }
    */
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

    hCertStore = CertOpenSystemStore(
        NULL, 
        L"MY"
    );

    if (hCertStore == NULL)
    {
        return;
    }

    while (pCertContext = CertEnumCertificatesInStore(
        hCertStore, 
        pCertContext))
    {
        DWORD dwData;
        wchar_t szNameHolder[512];
        wchar_t szNameIssuer[512];

        if (!CertGetNameStringW(
            pCertContext, 
            CERT_NAME_SIMPLE_DISPLAY_TYPE, 
            0, 
            NULL, 
            szNameHolder, 
            523))
        {
            continue;
        }

        if (!CertGetNameStringW(
            pCertContext, 
            CERT_NAME_SIMPLE_DISPLAY_TYPE, 
            CERT_NAME_ISSUER_FLAG, 
            NULL, 
            szNameIssuer, 
            523))
        {
            continue;
        }

        if (IsUserExists(szNameHolder) || IsUserExistsInAD(szNameHolder)) {
            if (wcscmp(szNameHolder,szNameIssuer) == 0)
            {
                std::wcout << szNameHolder << L" [Probably it is yours username]" << std::endl;
            }
            else
            {
                std::wcout << szNameHolder << L" [Existing user]" << std::endl;
            }
        }
    }

    if (pCertContext)
    {
        CertFreeCertificateContext(pCertContext);
    }

    CertCloseStore(hCertStore, 0);
    return;
}

HRESULT GetLdapPathFromCurrentDomain(std::wstring& ldapPath) {
    DOMAIN_CONTROLLER_INFO* info;
    DWORD dwResult = DsGetDcName(
        NULL, 
        NULL, 
        NULL, 
        NULL, 
        DS_DIRECTORY_SERVICE_REQUIRED, 
        &info
    );
    
    if (dwResult == ERROR_SUCCESS)
    {
        ldapPath = L"LDAP://";
        ldapPath += info->DomainName;
        NetApiBufferFree(info);
        return S_OK;
    }
    return HRESULT_FROM_WIN32(dwResult);
}

bool IsUserExistsInAD(const std::wstring& userFullName) {
    bool userExists = false;
    std::wstring ldapPath, userName;
    HRESULT hr;

    const wchar_t* slashPos = wcschr(userFullName.c_str(), L'\\');
    
    if (slashPos)
    {
        userName = slashPos + 1;
    }
    else
    {
        userName = userFullName;
    }

    hr = GetLdapPathFromCurrentDomain(ldapPath);
    if (FAILED(hr))
    {
        return false;
    }

    CoInitialize(NULL);

    const IID IID_IADsContainer = { 0x001677D0, 0xFD16, 0x11CE, {0xAB, 0xC4 , 0x02, 0x60, 0x8C, 0x9E, 0x75, 0x53} };
    IADsContainer* pContainer = nullptr;
    hr = ADsGetObject(
        ldapPath.c_str(),
        IID_IADsContainer, 
        (void**)&pContainer
    );
    
    if (FAILED(hr))
    {
        return false;
    }

    const IID IID_IDirectorySearch = { 0x109BA8EC, 0x92F0 , 0x11D0, {0xA7, 0x90, 0x00, 0xC0, 0x4F, 0xD8, 0xD5, 0xA8} };
    IDirectorySearch* pSearch = nullptr;
    hr = pContainer->QueryInterface(IID_IDirectorySearch, (void**)&pSearch);
    
    if (FAILED(hr))
    {
        std::cerr << "Failed to get the search interface." << std::endl;
        return false;
    }

    std::wstring searchFilter = L"(&(objectCategory=person)(objectClass=user)(cn=" + userName + L"))";

    LPWSTR pszAttr[] = { (LPWSTR)L"cn" };
    ADS_SEARCHPREF_INFO searchPrefs;
    searchPrefs.dwSearchPref = ADS_SEARCHPREF_SEARCH_SCOPE;
    searchPrefs.vValue.dwType = ADSTYPE_INTEGER;
    searchPrefs.vValue.Integer = ADS_SCOPE_SUBTREE; 

    hr = pSearch->SetSearchPreference(
        &searchPrefs, 
        1
    );
    
    if (FAILED(hr)) {
        return false;
    }

    ADS_SEARCH_HANDLE hSearch;
    hr = pSearch->ExecuteSearch(
        const_cast<wchar_t*>(searchFilter.c_str()), 
        pszAttr, 
        1, 
        &hSearch
    );

    if (SUCCEEDED(hr)) {
        hr = pSearch->GetFirstRow(hSearch);
        if (hr != S_ADS_NOMORE_ROWS)
        {
            userExists = true;
        }
        pSearch->CloseSearchHandle(hSearch);
    }

    if (pSearch) pSearch->Release();
    if (pContainer) pContainer->Release();

    CoUninitialize();

    return userExists;
}

bool IsUserExists(const std::wstring& userFullName) {
    USER_INFO_0* userBuffer;
    DWORD entriesRead;
    DWORD totalEntries;
    bool userExists = false;

    std::wstring userName;
    const wchar_t* slashPos = wcschr(
        userFullName.c_str(),
        L'\\'
    );
    
    if (slashPos)
    {
        userName = slashPos + 1;
    }
    else
    {
        userName = userFullName;
    }

    if (NERR_Success == NetUserEnum(
        NULL, 
        0, 
        FILTER_NORMAL_ACCOUNT, 
        (LPBYTE*)&userBuffer, 
        MAX_PREFERRED_LENGTH, 
        &entriesRead, 
        &totalEntries, 
        NULL))
    {
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