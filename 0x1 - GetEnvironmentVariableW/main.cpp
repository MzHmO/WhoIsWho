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
    username = (LPWSTR)malloc(size * sizeof(WCHAR));
    if (username == NULL)
    {
        return;
    }

    if (!GetEnvironmentVariableW(
		L"USERNAME",
		username,
		size))
    {
        free(username);
        return;
    }

    std::wcout << username << std::endl;

    free(username);
    return;
}