/*
 * Widescreen patch for Soldiers: Heroes of World War II by zocker_160
 *
 * This source code is licensed under GPLv3
 *
 */

#include "ResPatch.h"

#include "utilities/Helper/Helper.h"
#include "utilities/Helper/Logger.h"

#include <iostream>
#include <sstream>

DWORD TResLimit = 0xE79F6;
DWORD TResLimitEditor = 0x1D1C46;

DWORD CameraMaxH = 0x4FEDF4; // 100000 if not init
DWORD CameraMinH = 0x4FEDF0; // 1 if not init
DWORD CameraZoomStepBig = 0x46576C;
DWORD CameraZoomStepBigPTR = 0x39AD30;
DWORD CameraStatus = 0x4fede8;
DWORD GameLoadStatus = 0x4E0834;

/*###################################*/

bool fcmp(float a, float b) {
    return fabs(a - b) < FLT_EPSILON;
}

int MainEntry(threadData* tData) {
    Logging::Logger logger("MAIN");

    /* fix for "Texture or surface size is too big (esurface.cpp, 129)"  */
    int newResLimit;
    if (tData->bSuperUltrawide)
        newResLimit = 8192;
    else
        newResLimit = 4096;

    int* textureLimit_p = (int*)calcAddress(TResLimit);
    int* textureLimitEditor_p = (int*)calcAddress(TResLimitEditor);

    logger.debug() << "texture limit: " << *textureLimit_p << std::endl;

    // wait until value can be written (fix for Steam version)
    bool bPatched = false;
    for (int i = 0; i < RETRY_COUNT; i++) {
        if (*textureLimit_p == 2048) {
            logger.info("patching resolution limit for game...");
            writeBytes(textureLimit_p, &newResLimit, 4);
            bPatched = true;
            break;
        }
        if (*textureLimitEditor_p == 2048) {
            logger.info("patching resolution limit for editor...");
            writeBytes(textureLimitEditor_p, &newResLimit, 4);
            bPatched = true;
            return 0; // editor does not need camera patch
        }

        logger.info("unexpected value - retrying...");
        Sleep(200);
    }

    if (!bPatched) {
        logger.error("resolution limit could not be set - exiting");
        return 0;
    }

    logger.debug() << "texture limit: " << *textureLimit_p << std::endl;

    if ( !tData->bCameraPatch || (getAspectRatio() < calcAspectRatio(16, 9) && !tData->bEnforceCamPatch) ) {
        logger.info("ignoring camera patch and exit");
        return 0;
    }

    /* camera and zoom patch */
    float* minH = (float*)(calcAddress(CameraMinH));
    float* maxH = (float*)(calcAddress(CameraMaxH));
    float* camStat = (float*)(calcAddress(CameraStatus));
    float* zoomStepB = (float*)(calcAddress(CameraZoomStepBig));
    DWORD* zoomStepBPTR = calcAddress(CameraZoomStepBigPTR);
    float* newZoomStep_p = &tData->fZoomStepBig;

    for (;; Sleep(1000)) {
        if ( (!fcmp(*minH, 1.0f) || !fcmp(*maxH, 100000.f)) && !fcmp(*camStat, 0.0f) ) {
            if (*minH != tData->fMinHeight || *maxH != tData->fMaxHeight) {
                logger.debug("writing values...");
                writeBytes(minH, &tData->fMinHeight, 4);
                writeBytes(maxH, &tData->fMaxHeight, 4);

                logger.debug("writing zoom step pointer");
                writeBytes(zoomStepBPTR, &newZoomStep_p, 4);
            }
        }
    }

    return 0;
}

DWORD WINAPI PatchThread(LPVOID param) {
    return MainEntry(reinterpret_cast<threadData*>(param));
}
