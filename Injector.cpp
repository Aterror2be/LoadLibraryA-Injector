#include <Windows.h>
#include <TlHelp32.h>
#include <codecvt>
#include <string>

#include "Injector.hpp"

bool Injector::Inject(std::string process_name, std::string dll_path)
{
    DWORD process_id = GetPidFromName(process_name);
    if (!process_id)
        return false;

	HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, false, process_id);
	if (!process)
		return false;

	LPVOID allocated_block = VirtualAllocEx(process, nullptr, MAX_PATH, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	if (!allocated_block)
		return false;

	if (!WriteProcessMemory(process, allocated_block, dll_path.c_str(), MAX_PATH, nullptr))
		return false;

	HANDLE thread = CreateRemoteThread(process, nullptr, NULL, LPTHREAD_START_ROUTINE(LoadLibraryA), allocated_block, NULL, NULL);
	if (!thread)
		return false;

	CloseHandle(process);

	return true;
}

DWORD Injector::GetPidFromName(std::string process_name)
{
    wchar_t name[MAX_PATH] = {0};
    std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
    std::u16string converted_string = convert.from_bytes(process_name.c_str());
    memcpy(name, (void*)converted_string.c_str(), converted_string.size() * sizeof(wchar_t));

    HANDLE handle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
    if (handle == INVALID_HANDLE_VALUE)
        return false;

    PROCESSENTRY32W entry = { 0 };
    entry.dwSize = sizeof(PROCESSENTRY32W);

    DWORD pid = 0;
    if (Process32FirstW(handle, &entry))
    {
        if (!_wcsicmp(name, entry.szExeFile))
        {
            pid = entry.th32ProcessID;
        }
        else while (Process32NextW(handle, &entry))
        {
            if (!_wcsicmp(name, entry.szExeFile))
            {
                pid = entry.th32ProcessID;
            }
        }
    }
    CloseHandle(handle); 
    return pid;
}