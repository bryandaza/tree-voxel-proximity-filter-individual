#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <filesystem>
#include <cstdint>

struct CachedTree {
    std::string relPath;
    uint64_t fileSize = 0;
    int64_t mtime = 0;
    double minZ = 0.0, maxZ = 0.0;
    std::vector<uint64_t> keys;
};

struct VoxelCacheData {
    double voxelSize = 0.0;
    uint32_t version = 1;
    std::unordered_map<std::string, CachedTree> byRelPath;
};

bool loadVoxelCache(const std::filesystem::path& cacheFile, VoxelCacheData& out);
bool saveVoxelCache(const std::filesystem::path& cacheFile, const VoxelCacheData& cache);

uint64_t getFileSizeU64(const std::filesystem::path& p);
int64_t  getFileMTimeI64(const std::filesystem::path& p);
std::string makeRelPath(const std::filesystem::path& inputDir, const std::filesystem::path& file);
