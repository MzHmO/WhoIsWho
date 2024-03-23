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

void WhoIsWho()
{
	HANDLE hToken;
	if (!OpenProcessToken(
		GetCurrentProcess(),
		TOKEN_READ,
		&hToken))
	{
		return;
	}

	DWORD bufferSize = 0;
	if (!GetTokenInformation(
		hToken, 
		TokenUser, 
		NULL,
		0, 
		&bufferSize) &&
		GetLastError() != ERROR_INSUFFICIENT_BUFFER)
	{
		CloseHandle(hToken);
		return;
	}

	PTOKEN_USER pTokenUser = (PTOKEN_USER)malloc(bufferSize);
	if (!pTokenUser)
	{
		CloseHandle(hToken);
		return;
	}

	if (!GetTokenInformation(
		hToken, 
		TokenUser, 
		pTokenUser, 
		bufferSize, 
		&bufferSize)) 
	{
		free(pTokenUser);
		CloseHandle(hToken);
		return;
	}

	WCHAR accountName[MAX_PATH];
	WCHAR domainName[MAX_PATH];
	DWORD accountNameSize = MAX_PATH;
	DWORD domainNameSize = MAX_PATH;
	SID_NAME_USE snu;

	if (!LookupAccountSidW(
		NULL, 
		pTokenUser->User.Sid, 
		accountName, 
		&accountNameSize, 
		domainName, 
		&domainNameSize, 
		&snu))
	{
		free(pTokenUser);
		CloseHandle(hToken);
		return;
	}

	std::wcout << accountName << std::endl;

	free(pTokenUser);
	CloseHandle(hToken);

	return;
}