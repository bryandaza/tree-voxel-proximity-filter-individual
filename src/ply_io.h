#pragma once
#include <filesystem>
#include "voxel.h"

TreeVoxels loadAndVoxelizePLY(const std::filesystem::path& file, double voxelSize);
