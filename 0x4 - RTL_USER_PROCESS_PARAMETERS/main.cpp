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

#include <winternl.h>
#include <vector>
#pragma comment(lib, "ntdll.lib")
void WhoIsWho()
{
    int processparameters_offset, environmentsize_offset, environment_offset;
    if (sizeof(void*) == 8) {
        processparameters_offset = 0x20;
        environmentsize_offset = 0x3F0;
        environment_offset = 0x80;
    }
    else {
        processparameters_offset = 0x10;
        environmentsize_offset = 0x0290;
        environment_offset = 0x48;
    }
    HANDLE hProcess = GetCurrentProcess();
    PROCESS_BASIC_INFORMATION pbi;
    ULONG temp;

    NtQueryInformationProcess(hProcess, (PROCESSINFOCLASS)0, &pbi, sizeof(pbi), &temp);

    PVOID processparameters_pointer = (PBYTE)pbi.PebBaseAddress + processparameters_offset;
    PVOID processparameters;
    SIZE_T bytesRead;
    ReadProcessMemory(hProcess, processparameters_pointer, &processparameters, sizeof(PVOID), &bytesRead);

    PVOID environment_size_pointer = (PBYTE)processparameters + environmentsize_offset;
    ULONG_PTR environment_size;
    ReadProcessMemory(hProcess, environment_size_pointer, &environment_size, sizeof(ULONG_PTR), &bytesRead);

    PVOID environment_pointer = (PBYTE)processparameters + environment_offset;
    PVOID environment_start;
    ReadProcessMemory(hProcess, environment_pointer, &environment_start, sizeof(PVOID), &bytesRead);

    std::vector<char> data(environment_size);
    ReadProcessMemory(hProcess, environment_start, data.data(), data.size(), &bytesRead);

    std::wstring environment_vars(reinterpret_cast<wchar_t*>(data.data()), data.size() / sizeof(wchar_t));
    std::wstring::size_type start_pos = environment_vars.find(L"USERNAME=");
    if (start_pos != std::wstring::npos) {
        start_pos += wcslen(L"USERNAME=");
        auto end_pos = environment_vars.find(L'\0', start_pos);
        auto username = environment_vars.substr(start_pos, end_pos - start_pos);

        std::wcout << username << std::endl;
    }
    else {
        std::cout << "USERNAME variable not found." << std::endl;
    }
}
