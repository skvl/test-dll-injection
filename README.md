# Inject DLL with CreateRemoteThread

The application injects a DLL into the specified process. The DLL creates a thread with endless loop.

# Notes

* The full path of the DLL should be specified.
* The x86 application could not be used to inject a DLL into x64 process.
* The output file ${DESKTOP_PATH}/inject.txt contains single line in format "PID:TID".
