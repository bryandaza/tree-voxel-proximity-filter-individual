#pragma once
#include <filesystem>
#include <vector>
#include <string>
#include "voxel.h"

std::vector<TreeVoxels> buildTreesWithCache(const std::filesystem::path& inputDir,
                                           const std::vector<std::filesystem::path>& files,
                                           int threads,
                                           const std::string& cacheFile,
                                           bool useCache,
                                           double voxelSize,
                                           bool& cacheUpdated);
