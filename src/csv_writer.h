#pragma once
#include <string>
#include <vector>
#include <filesystem>
#include "voxel.h"
#include "individual_overlap.h"

bool writeIndividualCSV(const std::filesystem::path& outFile,
                        const std::vector<TreeVoxels>& trees,
                        const std::vector<HitRow>& hits,
                        int targetIdx,
                        double voxelSize,
                        int dilate);
