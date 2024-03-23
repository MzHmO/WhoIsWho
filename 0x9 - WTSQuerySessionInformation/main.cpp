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

#include <WtsApi32.h>
#pragma comment(lib, "Wtsapi32.lib")
void WhoIsWho()
{
    DWORD sessionId = WTSGetActiveConsoleSessionId();

    DWORD size = MAX_PATH;
    LPTSTR pUserName = (LPTSTR)malloc(size);
    if (WTSQuerySessionInformation(
        WTS_CURRENT_SERVER_HANDLE, 
        sessionId,
        WTSUserName, 
        &pUserName, 
        &size))
    { 

        std::wcout << pUserName << std::endl;

        WTSFreeMemory(pUserName);
    }

    return ;
}