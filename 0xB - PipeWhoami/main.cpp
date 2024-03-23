#include <windows.h>
#include <iostream>

void WhoIsWho();

int main()
{
    WhoIsWho();
    return 0;
}

void WhoIsWho() {
    SECURITY_ATTRIBUTES saAttr;
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;

    HANDLE g_hChildStd_OUT_Rd = NULL;
    HANDLE g_hChildStd_OUT_Wr = NULL;

    if (!CreatePipe(
        &g_hChildStd_OUT_Rd, 
        &g_hChildStd_OUT_Wr, 
        &saAttr, 
        0
    ))
        ExitProcess(1);

    if (!SetHandleInformation(
        g_hChildStd_OUT_Rd, 
        HANDLE_FLAG_INHERIT, 
        0
    ))
        ExitProcess(1);

    PROCESS_INFORMATION piProcInfo;
    ZeroMemory(
        &piProcInfo, 
        sizeof(PROCESS_INFORMATION)
    );

    STARTUPINFOA siStartInfo;
    ZeroMemory(
        &siStartInfo, 
        sizeof(STARTUPINFO)
    );
    siStartInfo.cb = sizeof(STARTUPINFO);
    siStartInfo.hStdOutput = g_hChildStd_OUT_Wr;
    siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

    char cmd[] = "whoami.exe"; 

    if (!CreateProcessA(
        NULL, 
        cmd, 
        NULL, 
        NULL, 
        TRUE, 
        0, 
        NULL, 
        NULL, 
        &siStartInfo, 
        &piProcInfo))
    {
        std::cerr << "CreateProcess failed (" << GetLastError() << ").\n";
        return;
    }

    CloseHandle(g_hChildStd_OUT_Wr);

    DWORD dwRead;
    CHAR chBuf[4096];

    while (true)
    {
        bool success = ReadFile(
            g_hChildStd_OUT_Rd, 
            chBuf, 
            sizeof(chBuf), 
            &dwRead, 
            NULL
        );
        if (!success || dwRead == 0)
            break;

        std::string output(chBuf, dwRead);
        std::cout << output;
    }

    CloseHandle(g_hChildStd_OUT_Rd);
    CloseHandle(piProcInfo.hProcess);
    CloseHandle(piProcInfo.hThread);

    return;
}
