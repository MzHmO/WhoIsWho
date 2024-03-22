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

#include <NTSecAPI.h>
#pragma comment(lib, "advapi32.lib")
void WhoIsWho()
{
    HANDLE hToken;

    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
        return;
    }

    PTOKEN_USER pTokenUser = nullptr;
    DWORD TokenInformationLength = 0;

    if (!GetTokenInformation(hToken, TokenUser, NULL, 0, &TokenInformationLength) &&
        GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
        return;
    }

    pTokenUser = (PTOKEN_USER)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, TokenInformationLength);
    if (pTokenUser == NULL) {
        return;
    }

    if (!GetTokenInformation(hToken, TokenUser, pTokenUser, TokenInformationLength, &TokenInformationLength)) {
        return;
    }

    LSA_OBJECT_ATTRIBUTES lsaAttr;
    ZeroMemory(&lsaAttr, sizeof(LSA_OBJECT_ATTRIBUTES));
    LSA_HANDLE hLsaPolicy = NULL;
    NTSTATUS status = LsaOpenPolicy(NULL, &lsaAttr, POLICY_LOOKUP_NAMES, &hLsaPolicy);
    if (status != 0) {
        return;
    }

    PLSA_REFERENCED_DOMAIN_LIST pDomains = NULL;
    PLSA_TRANSLATED_NAME pNames = NULL;
    status = LsaLookupSids(hLsaPolicy, 1, &(pTokenUser->User.Sid), &pDomains, &pNames);
    if (status != 0) {
        LsaClose(hLsaPolicy);
        return;
    }

    std::wcout << pNames[0].Name.Buffer << std::endl;

    if (pTokenUser) HeapFree(GetProcessHeap(), 0, pTokenUser);
    LsaFreeMemory(pDomains);
    LsaFreeMemory(pNames);
    LsaClose(hLsaPolicy);
    CloseHandle(hToken);
	return;
}