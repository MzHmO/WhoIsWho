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

#include <userenv.h>
#pragma comment(lib, "Userenv.lib")
void WhoIsWho() {
	HANDLE hToken;
	if (!OpenProcessToken(
		GetCurrentProcess(),
		TOKEN_READ,
		&hToken))
	{
		return;
	}

	DWORD size;
	LPWSTR lpProfileDir = NULL;
	if (GetUserProfileDirectoryW(
		hToken,
		lpProfileDir,
		&size
	) != 0)
	{
		return;
	}

	lpProfileDir = (LPWSTR)malloc(size * sizeof(WCHAR));

	if (!GetUserProfileDirectoryW(
		hToken,
		lpProfileDir,
		&size
	))
	{
		return;
	}

	std::wcout << lpProfileDir << std::endl;

	free(lpProfileDir);
    
    return;
}