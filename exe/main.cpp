#include <cstring>
#include <iostream>
#include <string>

#include <Windows.h>
#include <TlHelp32.h>


DWORD get_process_id(std::string process_name)
{
    DWORD process_id = 0;
    HANDLE snapshot;
    PROCESSENTRY32 process_entry = {0};

    if( (snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0)) == INVALID_HANDLE_VALUE )
    {
        return 0;
    }

    process_entry.dwSize = sizeof(PROCESSENTRY32);
    Process32First(snapshot, &process_entry);
    do
    {
        if ( std::string(process_entry.szExeFile) == process_name )
            return process_entry.th32ProcessID;
    } while (Process32Next(snapshot,&process_entry));

    if ( snapshot != INVALID_HANDLE_VALUE )
        CloseHandle(snapshot);

    return 0;
} 


int main(int argc, char** argv)
{
    if (argc != 2 && argc != 3)
    {
        std::cout << "Usage: " << argv[0] << " DLL [PROCESS]" << std::endl;
        return 1;
    }

    std::string lib_path(argv[1]);
    std::string process_name("explorer.exe");
    if (argc == 3)
        process_name = std::string(argv[2]);

    auto kernel32 = GetModuleHandle("Kernel32");
    if (nullptr == kernel32)
    {
        std::cout << "Failed to get module handle" << std::endl;
        return 2;
    }

    auto entry_point = GetProcAddress(kernel32, "LoadLibraryA");
    if (nullptr == kernel32)
    {
        std::cout << "Failed to get procedure address" << std::endl;
        return 3;
    }

    auto pid = get_process_id(process_name);
    if (0 == pid)
    {
        std::cout << "Failed to get target PID" << std::endl;
        return 4;
    }

    std::cout << "Get process PID: " << pid << std::endl;

    auto target = OpenProcess(PROCESS_ALL_ACCESS, false, pid);
    if (nullptr == target)
    {
        std::cout << "Failed to open process" << std::endl;
        return 5;
    }

    auto remote_buffer = VirtualAllocEx( target, nullptr, lib_path.size(), MEM_COMMIT, PAGE_READWRITE );
    if (nullptr == remote_buffer)
    {
        CloseHandle(target);
        std::cout << "Failed to allocate remote memory" << std::endl;
        return 6;
    }

    WriteProcessMemory( target, remote_buffer, (void*)lib_path.data(), lib_path.size(), nullptr );

    auto remote_thread = CreateRemoteThread( target, nullptr, 0, (LPTHREAD_START_ROUTINE) entry_point, remote_buffer, 0, nullptr );
    if (nullptr == remote_thread)
    {
        VirtualFreeEx(target, remote_buffer, lib_path.size(), MEM_DECOMMIT);
        CloseHandle(target);
        std::cout << "Failed to create remote thread" << std::endl;
        return 7;
    }

    // NOTE Don't free memory to avoid null pointer dereference in remote
    // VirtualFreeEx(target, remote_buffer, lib_path.size(), MEM_DECOMMIT);
    CloseHandle(target);

    return 0;
}
