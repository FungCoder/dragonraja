#pragma once

#include "mytypes.h"

enum AnimationTableLoadResult
{
    ANIMATION_TABLE_OK,
    ANIMATION_TABLE_MISSING,
    ANIMATION_TABLE_INVALID
};

AnimationTableLoadResult LoadAnimationTableBinary(
    const char* file_name, PCANIMATIONTABLE* destination, int capacity);
