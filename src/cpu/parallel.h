#pragma once
#include <stdint.h>

#include "maths.h"

void InitializeTest();
void ShutdownTest();
void DrawTest(float time, int frameCount, int screenWidth, int screenHeight, float* backbuffer, int& outRayCount);