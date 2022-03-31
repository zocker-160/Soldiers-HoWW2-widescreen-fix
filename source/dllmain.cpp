/*
 * Widescreen patch for Soldiers: Heroes of World War II by zocker_160
 *
 * This source code is licensed under GPLv3
 *
 */

#include "pch.h"
#include "ResPatch.h"

bool bResPatch;
char iniPath[MAX_PATH];
char dllPath[MAX_PATH];

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     ){

    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    {
        //HMODULE hm = NULL;
        //GetModuleFileNameA(hm, iniPath, sizeof(iniPath));
        GetCurrentDirectoryA(MAX_PATH, iniPath);
        //*strrchr(iniPath, '\\') = '\0';
        strcat_s(iniPath, "\\WidescreenFix.ini");

        //MessageBoxA(NULL, iniPath, "Message from ZoomPatch by zocker_160", MB_OK);

        /* read settings from ini */

        threadData* tData = new threadData();
        bResPatch = GetPrivateProfileIntA("Patches", "ResolutionPatch", 1, iniPath) != 0;
        tData->bDebugMode = GetPrivateProfileIntA("Patches", "DebugMode", 0, iniPath) != 0;
        tData->bCameraPatch = GetPrivateProfileIntA("Patches", "CameraPatch", 1, iniPath) != 0;
        tData->fMinHeight = static_cast<float>(GetPrivateProfileIntA("Patches", "minHeight", 750, iniPath));
        tData->fMaxHeight = static_cast<float>(GetPrivateProfileIntA("Patches", "maxHeight", 1200, iniPath));
        tData->fZoomStepBig = static_cast<float>(GetPrivateProfileIntA("Patches", "zoomStep_big", 50, iniPath));

        // ResPatch
        if (bResPatch)
            CreateThread(0, 0, PatchThread, tData, 0, 0);

        break;
    }
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
