
#include <Windows.h>

int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, char*, int nShowCmd){
    STARTUPINFOW si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    bool ret = CreateProcessW(
        L"Binaries/HexGenerator.exe",
        nullptr,
        nullptr,
        nullptr,
        false,
        0,
        nullptr,
        nullptr,
        &si,
        &pi
    );
    if (!ret){
        MessageBoxW(nullptr, L"Unable to open: Binaries/HexGenerator.exe", L"Error", MB_ICONERROR);
        return 1;
    }
    ret = WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return ret;
}

