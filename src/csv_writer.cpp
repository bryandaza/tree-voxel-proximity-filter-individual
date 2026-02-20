#include "csv_writer.h"
#include <fstream>

namespace fs = std::filesystem;

static inline std::string baseName(const std::string& pathStr) {
    return fs::path(pathStr).filename().string();
}

bool writeIndividualCSV(const fs::path& outFile,
                        const std::vector<TreeVoxels>& trees,
                        const std::vector<HitRow>& hits,
                        int targetIdx,
                        double voxelSize,
                        int dilate)
{
    fs::create_directories(outFile.parent_path());

    std::ofstream out(outFile);
    if (!out) return false;

    // CSV header
    out << "target_file,target_id,target_height_m,"
           "other_file,other_id,other_height_m,"
           "hits,voxel_m,dilate\n";

    const auto& T = trees[targetIdx];
    const std::string tName = baseName(T.file);
    const double tH = T.height();

    for (const auto& h : hits) {
        const auto& O = trees[h.otherIdx];
        out << tName << "," << targetIdx << "," << tH << ","
            << baseName(O.file) << "," << h.otherIdx << "," << O.height() << ","
            << h.hits << "," << voxelSize << "," << dilate << "\n";
    }
    return true;
}
