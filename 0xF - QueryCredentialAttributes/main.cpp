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

#define SECURITY_WIN32
#include <sspi.h>
#pragma comment (lib, "Secur32.lib")
void WhoIsWho() {
	CredHandle hCredentials;
	TimeStamp tsExpires;
	NTSTATUS status = AcquireCredentialsHandle(
		NULL, 
		(LPWSTR)L"Kerberos",
		SECPKG_CRED_BOTH, 
		NULL, 
		NULL,
		NULL,
		NULL,
		&hCredentials,
		&tsExpires
	);
	
	if (status != SEC_E_OK)
	{
		status = AcquireCredentialsHandle(
			NULL,
			(LPWSTR)L"NTLM", 
			SECPKG_CRED_BOTH,
			NULL,
			NULL,
			NULL,
			NULL,
			&hCredentials,
			&tsExpires
		);

		if (status != SEC_E_OK)
		{
			return;
		}
	}
	
	SecPkgCredentials_NamesW names;
	status = QueryCredentialsAttributesW(
		&hCredentials,
		SECPKG_CRED_ATTR_NAMES,
		&names
	);

	if (status != SEC_E_OK)
	{
		return;
	}

	std::wcout << names.sUserName << std::endl;
	return;
}