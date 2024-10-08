#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <thread>
#include <vector>
#include <cstdlib>

// compile this in a seperate project
// i saw a random guy on unknonw cheats with this shit i think dyn or someone leaked this cus i wrote it myself!!!

DWORD GetProcessIdByName(const wchar_t* processName) {
    DWORD processID = 0;
    PROCESSENTRY32W processEntry;
    processEntry.dwSize = sizeof(PROCESSENTRY32W);

    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) return 0;

    if (Process32FirstW(hSnapshot, &processEntry)) {
        do {
            if (wcscmp(processEntry.szExeFile, processName) == 0) {
                processID = processEntry.th32ProcessID;
                break;
            }
        } while (Process32NextW(hSnapshot, &processEntry));
    }

    CloseHandle(hSnapshot);
    return processID;
}

void HookFunction(HANDLE processHandle, LPVOID functionAddress, const std::vector<BYTE>& newBytes, std::vector<BYTE>& originalBytes) {
    SIZE_T bytesWritten;

    ReadProcessMemory(processHandle, functionAddress, originalBytes.data(), originalBytes.size(), NULL);

    WriteProcessMemory(processHandle, functionAddress, newBytes.data(), newBytes.size(), &bytesWritten);
    std::cout << "[+] Hooked function at: " << functionAddress << std::endl;
}

void RestoreFunction(HANDLE processHandle, LPVOID functionAddress, const std::vector<BYTE>& originalBytes) {
    SIZE_T bytesWritten;
    WriteProcessMemory(processHandle, functionAddress, originalBytes.data(), originalBytes.size(), &bytesWritten);
    std::cout << "[+] Restored function at: " << functionAddress << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <dll> <callback> <optional pid>" << std::endl;
        std::cin.get();
        return -1;
    }

    const wchar_t* targetProcessName = L"RobloxPlayerBeta.exe";
    const char* dllPath = argv[1];
    const char* callBackName = argv[2];
    int pid = std::atoi(argv[3]);

    DWORD processID = 0;
    if (pid) {
        processID = pid;
    }
    else {
        while (processID == 0) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            processID = GetProcessIdByName(targetProcessName);
        }
    }

    HANDLE processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processID);
    if (!processHandle) {
        std::cerr << "[-] Failed to open process." << std::endl;
        std::cin.get();
        return -1;
    }

    std::cout << "[+] Opened process: " << processID << std::endl;

    HMODULE hModule = LoadLibraryA("C:\\Windows\\System32\\Wintrust.dll");
    if (!hModule) {
        std::cerr << "[-] Failed to load Wintrust.dll." << std::endl;
        std::cin.get();
        CloseHandle(processHandle);
        return -1;
    }

    FARPROC funcAddress = GetProcAddress(hModule, "WinVerifyTrust");
    if (!funcAddress) {
        std::cerr << "[-] Failed to find WinVerifyTrust." << std::endl;
        std::cin.get();
        CloseHandle(processHandle);
        return -1;
    }

    std::cout << "[+] WinVerifyTrust address: " << funcAddress << std::endl;

    std::vector<BYTE> originalBytes(6);
    std::vector<BYTE> hookBytes = { 0xB8, 0x00, 0x00, 0x00, 0x00, 0xC3 };

    HookFunction(processHandle, (LPVOID)funcAddress, hookBytes, originalBytes);

    HWND hwnd = FindWindowA(NULL, "Roblox");
    DWORD threadID = GetWindowThreadProcessId(hwnd, NULL);

    HMODULE dll = LoadLibraryExA(dllPath, NULL, DONT_RESOLVE_DLL_REFERENCES);
    if (!dll) {
        std::cerr << "[-] Failed to load " << dllPath << std::endl;
        std::cin.get();
        RestoreFunction(processHandle, (LPVOID)funcAddress, originalBytes);
        CloseHandle(processHandle);
        return -1;
    }

    FARPROC callbackAddr = GetProcAddress(dll, callBackName);
    if (!callbackAddr) {
        std::cerr << "[-] Invalid DLL or callback not found." << std::endl;
        std::cin.get();
        RestoreFunction(processHandle, (LPVOID)funcAddress, originalBytes);
        CloseHandle(processHandle);
        return -1;
    }

    HHOOK hook = SetWindowsHookExA(WH_GETMESSAGE, (HOOKPROC)callbackAddr, dll, threadID);
    if (!hook) {
        std::cerr << "[-] Failed to set Windows hook." << std::endl;
        std::cin.get();
        RestoreFunction(processHandle, (LPVOID)funcAddress, originalBytes);
        CloseHandle(processHandle);
        return -1;
    }

    std::cout << "[+] Windows hook set successfully." << std::endl;

    PostThreadMessageA(threadID, WM_NULL, 0, 0);

    /* Unhooking and cleaning up code
    UnhookWindowsHookEx(hook);
    std::cout << "[+] Windows hook removed." << std::endl;

    RestoreFunction(processHandle, (LPVOID)funcAddress, originalBytes);

    CloseHandle(processHandle);
    FreeLibrary(hModule);
    FreeLibrary(dll);
    */

    std::cout << "[+] Done." << std::endl;
    system("pause");
    FreeConsole();
    return 0;
}
