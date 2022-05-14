/*
 * Widescreen patch for Soldiers: Heroes of World War II by zocker_160
 *
 * This source code is licensed under GPLv3
 *
 */

#pragma once
#include "pch.h"

struct memoryPTR {
    DWORD base_address;
    int total_offsets;
    int offsets[];
};

struct threadData {
	bool bDebugMode;
    bool bCameraPatch;
    bool bEnforceCamPatch;
    bool bSuperUltrawide;
    float fMinHeight;
    float fMaxHeight;
    float fZoomStepBig;
};

const int version_maj = 1;
const int version_min = 6;

const int RETRY_COUNT = 20;

DWORD WINAPI PatchThread(LPVOID param);
