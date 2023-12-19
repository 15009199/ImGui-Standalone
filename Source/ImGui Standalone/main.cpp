#include "UI.h"

#ifdef _WINDLL
HANDLE hCurrentUIThread = nullptr;

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved) {
    if (fdwReason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hinstDLL);
        UI::hCurrentModule = hinstDLL;
        hCurrentUIThread = CreateThread(nullptr, NULL, (LPTHREAD_START_ROUTINE)UI::Render, nullptr, NULL, nullptr);
        MyLogger::LogHRESULT(S_OK, "UI Rendering thread created", __FILE__, __LINE__);
    }

    if (fdwReason == DLL_PROCESS_DETACH) {
        TerminateThread(hCurrentUIThread, 0);
        MyLogger::LogHRESULT(S_OK, "UI Rendering thread terminated", __FILE__, __LINE__);
    }

    return TRUE;
}
#else
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd) {
    UI::Render();
    MyLogger::LogHRESULT(S_OK, "UI Rendering in wWinMain", __FILE__, __LINE__);
    return 0;
}
#endif
