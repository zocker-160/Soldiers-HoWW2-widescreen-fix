/*
 * Widescreen patch for Soldiers: Heroes of World War 2 by zocker_160
 *
 * This source code is licensed under GPL-v3
 *
 */
#include "ResPatch.h"
#include <Windows.h>
#include <iostream>
#include <sstream>

DWORD TextureResolutionLimit = 0xE79F6;
DWORD CameraMaxH = 0x4FEDF4; // 100000 if not init
DWORD CameraMinH = 0x4FEDF0; // 1 if not init
DWORD CameraZoomStepBig = 0x46576C;
DWORD CameraZoomStepBigPTR = 0x39AD30;
DWORD CameraStatus = 0x4fede8;
DWORD GameLoadStatus = 0x4E0834;

/*###################################*/

// reading and writing stuff / helper functions and other crap

/* update memory protection and read with memcpy */
void protectedRead(void* dest, void* src, int n) {
    DWORD oldProtect = 0;
    VirtualProtect(dest, n, PAGE_EXECUTE_READWRITE, &oldProtect);
    memcpy(dest, src, n);
    VirtualProtect(dest, n, oldProtect, &oldProtect);
}
/* read from address into read buffer of length len */
bool readBytes(void* read_addr, void* read_buffer, int len) {
    // compile with "/EHa" to make this work
    // see https://stackoverflow.com/questions/16612444/catch-a-memory-access-violation-in-c
    try {
        protectedRead(read_buffer, read_addr, len);
        return true;
    }
    catch (...) {
        return false;
    }
}
/* write patch of length len to destination address */
void writeBytes(void* dest_addr, void* patch, int len) {
    protectedRead(dest_addr, patch, len);
}

/* fiddle around with the pointers */
HMODULE getBaseAddress() {
    return GetModuleHandle(NULL);
}
DWORD* calcAddress(DWORD appl_addr) {
    return (DWORD*)((DWORD)getBaseAddress() + appl_addr);
}
DWORD* tracePointer(memoryPTR* patch) {
    DWORD* location = calcAddress(patch->base_address);

    for (int i = 0; i < patch->total_offsets; i++) {
        location = (DWORD*)(*location + patch->offsets[i]);
    }
    return location;
}

void GetDesktopResolution(int& hor, int& vert) {
    hor = GetSystemMetrics(SM_CXSCREEN);
    vert = GetSystemMetrics(SM_CYSCREEN);
}
float calcAspectRatio(int horizontal, int vertical) {
    if (horizontal != 0 && vertical != 0) {
        return (float)horizontal / (float)vertical;
    }
    else {
        return -1.0f;
    }
}
float getAspectRatio() {
    int horizontal, vertical;
    GetDesktopResolution(horizontal, vertical);
    return calcAspectRatio(horizontal, vertical);
}

/* other helper functions and stuff */
void showMessage(float val) {
    std::cout << "DEBUG: " << val << "\n";
}
void showMessage(int val) {
    std::cout << "DEBUG: " << val << "\n";
}
void showMessage(LPCSTR val) {
    std::cout << "DEBUG: " << val << "\n";
}
void startupMessage() {
    std::cout << "Resolution Patch by zocker_160 - Version: v" << version_maj << "." << version_min << "\n";
    std::cout << "Debug mode enabled!\n";
    std::cout << "Waiting for application startup...\n";
}

bool fcmp(float a, float b) {
    return fabs(a - b) < FLT_EPSILON;
}

int MainEntry(threadData* tData) {
    FILE* f;

    if (tData->bDebugMode) {
        AllocConsole();
        freopen_s(&f, "CONOUT$", "w", stdout);
        startupMessage();
    }

    /* fix for "Texture or surface size is too big (esurface.cpp, 129)"  */
    int newResLimit = 4096;
    int* textureLimit_p = (int*)(calcAddress(TextureResolutionLimit));

    showMessage(*textureLimit_p);
    if (*textureLimit_p == 2048)
        writeBytes(textureLimit_p, &newResLimit, 4);
    else
        showMessage("Unexpected value - ResPatch skipped");

    showMessage(*textureLimit_p);

    if (!tData->bCameraPatch || getAspectRatio() < calcAspectRatio(16, 9)) {
        showMessage("ignoring camera patch and exit");
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
                showMessage("writing values...");
                writeBytes(minH, &tData->fMinHeight, 4);
                writeBytes(maxH, &tData->fMaxHeight, 4);

                showMessage("writing zoom step pointer");
                writeBytes(zoomStepBPTR, &newZoomStep_p, 4);
            }
        }
    }

    return 0;
}

DWORD WINAPI PatchThread(LPVOID param) {
    return MainEntry(reinterpret_cast<threadData*>(param));
}
