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

    if (GetUserNameW(
		NULL, 
		&size) != 0)
    {
        return;
    }

    username = (LPWSTR)malloc(size * sizeof(WCHAR));
	
    if (username == NULL)
    {
        return;
    }

    if (!GetUserNameW(
		username, 
		&size))
    {
        free(username);
        return;
    }

    std::wcout << username << std::endl;

    free(username);
    return;
}