/*
 * Widescreen patch for Soldiers: Heroes of World War II by zocker_160
 *
 * This source code is licensed under GPLv3
 *
 */

#include "ResPatch.h"

#include "utilities/Helper/Helper.h"
#include "utilities/Helper/Logger.h"

#define V_GOG "c46dbef054fcfb9d0ee5df2145763e1a0ea3fcb684df4892f1e63c48a5b5d951"
#define V_GOG_EDITOR "be404895472ecb2a33ae6e110a0cba17ddce9aad1fe9cb311cbd5d05e0356be9"
#define V_ZOOM "c46dbef054fcfb9d0ee5df2145763e1a0ea3fcb684df4892f1e63c48a5b5d951"
#define V_STEAM "839ae59bfd3ce356820e90fe0c648d4ec175686fa48c9de76619a8847f3e7254" // original pre patch version


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     ){

    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    {
        HMODULE hm = getBaseModule();

        char logfile[MAX_PATH];
        getGameDirectory(hm, logfile, MAX_PATH, "\\WidescreenFix.log");

        Logging::Logger logger("DLL", logfile);
        logger.info() << "Resolution Patch by zocker_160 - version: "
            << "v" << version_maj << "." << version_min << std::endl;

        char mainExecutable[MAX_PATH];
        GetModuleFileNameA(hm, mainExecutable, MAX_PATH);

        std::string checksum;
        if (!getFileChecksum(mainExecutable, checksum)) {
            logger.error("failed to calculate checksum");
            return 0;
        }

        bool supportedVersion = checksum == V_GOG || checksum == V_GOG_EDITOR 
            || checksum == V_ZOOM 
            || checksum == V_STEAM;

        if (!supportedVersion) {
            logger.error("unsupported game version / bailing out");
            return 0;
        }

        char iniPath[MAX_PATH];
        getGameDirectory(hm, iniPath, MAX_PATH, "\\WidescreenFix.ini");

        /* read settings from ini */

        threadData* tData = new threadData();
        bool bResPatch = GetPrivateProfileIntA("Patches", "ResolutionPatch", 1, iniPath) != 0;
        tData->bDebugMode = GetPrivateProfileIntA("Debug", "DebugMode", 0, iniPath) != 0;
        tData->bCameraPatch = GetPrivateProfileIntA("Patches", "CameraPatch", 1, iniPath) != 0;
        tData->bEnforceCamPatch = GetPrivateProfileIntA("Debug", "enforceCamPatch", 0, iniPath) != 0;
        tData->bSuperUltrawide = GetPrivateProfileIntA("Patches", "superUltrawideSupport", 0, iniPath) != 0;
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
