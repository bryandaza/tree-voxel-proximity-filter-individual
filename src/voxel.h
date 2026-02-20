#pragma once
#include <string>
#include <vector>
#include <cstdint>

struct TreeVoxels {
    std::string file;
    std::vector<uint64_t> keys; // packed voxels
    double minZ = 0.0;
    double maxZ = 0.0;

    double height() const { return maxZ - minZ; }
};
