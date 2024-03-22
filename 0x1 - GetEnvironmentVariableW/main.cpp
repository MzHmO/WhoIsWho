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
	DWORD size = 256;
	LPWSTR username = NULL;
	username = (LPWSTR)malloc(size);
	GetEnvironmentVariableW(L"USERNAME", username, size);
	DWORD dw = GetLastError();
	std::wcout << username << std::endl;
    	return;
}