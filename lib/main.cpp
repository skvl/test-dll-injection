#include <string>

#include <Shlobj.h>
#include <Windows.h>


extern "C" __declspec(dllexport) int wait_loop()
{
    while (true)
        Sleep(10);

    return 0;
}

static bool created = false;

static std::string get_path()
{
    static char buffer[MAX_PATH+1];
    std::string path("C:");

    if (S_OK == SHGetFolderPathA(NULL, CSIDL_DESKTOP, NULL, 0, buffer))
        path = std::string(buffer);

    return path + "\\inject.txt";
}

BOOL APIENTRY DllMain( HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved )
{
    if (created)
        return TRUE;

    DWORD thread_id = 0;
    auto thread = CreateThread( nullptr, 0, (LPTHREAD_START_ROUTINE) wait_loop, nullptr, 0, &thread_id );
    if (nullptr == thread)
        return FALSE;

    created = true;

    // Open a handle to the file
    HANDLE hFile = CreateFileA(
        get_path().data(),      // Filename
        GENERIC_WRITE,          // Desired access
        FILE_SHARE_READ,        // Share mode
        NULL,                   // Security attributes
        CREATE_ALWAYS,          // Creates a new file, only if it doesn't already exist
        FILE_ATTRIBUTE_NORMAL,  // Flags and attributes
        NULL);                  // Template file handle

     if (hFile == INVALID_HANDLE_VALUE)
        return TRUE; // NOTE To avoid unloading the library

    DWORD pid = GetCurrentProcessId();
    // Write data to the file
    std::string strText = std::to_string(pid) + ":" + std::to_string(thread_id); // For C use LPSTR (char*) or LPWSTR (wchar_t*)
    DWORD bytes_written = 0;
    WriteFile(
        hFile,            // Handle to the file
        strText.data(),   // Buffer to write
        strText.size(),   // Buffer size
        &bytes_written,   // Bytes written
        nullptr);         // Overlapped

    // Close the handle once we don't need it.
    CloseHandle(hFile);

    return TRUE;
}
