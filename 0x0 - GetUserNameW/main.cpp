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
	DWORD size = 0;
	LPWSTR username = NULL;
	GetUserNameW(NULL, &size);
	username = (LPWSTR)malloc(size + 1);
	GetUserNameW(username, &size);
    	std::wcout << username << std::endl;
    	return;
}