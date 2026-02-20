#pragma once
#include <vector>
#include <cstdint>
#include "voxel.h"

struct HitRow {
    int targetIdx;
    int otherIdx;
    uint32_t hits;
};

std::vector<HitRow> findTouchingForOne(const std::vector<TreeVoxels>& trees,
                                       int targetIdx,
                                       int dilate);
