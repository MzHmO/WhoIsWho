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

DWORD ThreadStartRoutine(LPVOID lpParameter);
void WhoIsWho()
{
	PWCHAR username = (PWCHAR)malloc(MAX_PATH);
	HANDLE hNamedPipe = NULL;
	HANDLE hThread = NULL;
	BOOL Connected = FALSE;
	typedef ULONG(WINAPI* NPGETUSERNAME)(HANDLE, LPWSTR, DWORD);

	DWORD UserNameLength = MAX_PATH;

	HMODULE hMod = NULL;
	NPGETUSERNAME NpGetUserName = NULL;
	WCHAR Buffer[5] = { 0 };
	DWORD dwRead = 0;

	hMod = LoadLibraryW(L"advapi32.dll");
	if (hMod == NULL)
		goto EXIT_ROUTINE;

	NpGetUserName = (NPGETUSERNAME)GetProcAddress(hMod, "NpGetUserName");
	if (!NpGetUserName)
		goto EXIT_ROUTINE;

	hNamedPipe = CreateNamedPipeW(L"\\\\.\\pipe\\LOCAL\\usernamepipe", PIPE_ACCESS_DUPLEX, PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT, PIPE_UNLIMITED_INSTANCES, 256, 256, 0, NULL);
	if (hNamedPipe == INVALID_HANDLE_VALUE)
		goto EXIT_ROUTINE;

	hThread = CreateThread(NULL, 0, ThreadStartRoutine, hNamedPipe, 0, NULL);
	if (hThread == NULL)
		goto EXIT_ROUTINE;

	if (!ConnectNamedPipe(hNamedPipe, NULL))
		goto EXIT_ROUTINE;
	else
		Connected = TRUE;

	WaitForSingleObject(hThread, INFINITE);

	if (!ReadFile(hNamedPipe, &Buffer, 5, &dwRead, NULL))
		goto EXIT_ROUTINE;

	NpGetUserName(hNamedPipe, username, UserNameLength);
	std::wcout << username << std::endl;

EXIT_ROUTINE:

	if (Connected)
		DisconnectNamedPipe(hNamedPipe);

	if (hNamedPipe)
		CloseHandle(hNamedPipe);

	return;
}

DWORD ThreadStartRoutine(LPVOID lpParameter)
{
	HANDLE hHandle = (HANDLE)lpParameter;
	HANDLE hPipe = INVALID_HANDLE_VALUE;

	DWORD dwSize = 0;
	WCHAR Buffer[] = L"Test";

	while (hHandle == NULL)
	{
		if (hHandle == INVALID_HANDLE_VALUE)
			goto EXIT_ROUTINE;
		else
			Sleep(1000);
	}

	hPipe = CreateFileW(L"\\\\.\\pipe\\LOCAL\\usernamepipe", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (hPipe == INVALID_HANDLE_VALUE)
		goto EXIT_ROUTINE;

	WriteFile(hPipe, L"test", 5, &dwSize, NULL);

EXIT_ROUTINE:

	if (hPipe)
		CloseHandle(hPipe);

	return 0;
}
