#include "build_trees_cache.h"
#include "voxel_cache.h"
#include "ply_io.h"

#include <future>
#include <atomic>
#include <mutex>
#include <iostream>

namespace fs = std::filesystem;

std::vector<TreeVoxels> buildTreesWithCache(const fs::path& inputDir,
                                           const std::vector<fs::path>& files,
                                           int threads,
                                           const std::string& cacheFile,
                                           bool useCache,
                                           double voxelSize,
                                           bool& cacheUpdated)
{
    cacheUpdated = false;

    fs::path cachePath = cacheFile;
    if (cachePath.is_relative()) cachePath = inputDir / cachePath;

    VoxelCacheData cache;
    if (useCache) {
        const bool ok = loadVoxelCache(cachePath, cache);
        if (!ok || cache.voxelSize != voxelSize) {
            cache = {};
            cache.voxelSize = voxelSize;
        }
    }

    std::vector<TreeVoxels> trees(files.size());
    const int T = std::max(1, threads);

    std::atomic<size_t> next{0};
    std::vector<std::future<void>> workers;
    workers.reserve(T);

    std::mutex cacheMutex;

    for (int t = 0; t < T; ++t) {
        workers.push_back(std::async(std::launch::async, [&](){
            while (true) {
                size_t i = next.fetch_add(1);
                if (i >= files.size()) break;

                const auto& f = files[i];

                if (!useCache) {
                    trees[i] = loadAndVoxelizePLY(f, voxelSize);
                    continue;
                }

                const std::string rel = makeRelPath(inputDir, f);
                const uint64_t fsize = getFileSizeU64(f);
                const int64_t  mtime = getFileMTimeI64(f);

                auto it = cache.byRelPath.find(rel);
                if (it != cache.byRelPath.end() &&
                    it->second.fileSize == fsize &&
                    it->second.mtime == mtime)
                {
                    TreeVoxels tv;
                    tv.file = f.string();
                    tv.keys = it->second.keys;
                    tv.minZ = it->second.minZ;
                    tv.maxZ = it->second.maxZ;
                    trees[i] = std::move(tv);
                    continue;
                }

                TreeVoxels tv = loadAndVoxelizePLY(f, voxelSize);
                trees[i] = tv;

                std::lock_guard<std::mutex> lk(cacheMutex);
                CachedTree e;
                e.relPath = rel;
                e.fileSize = fsize;
                e.mtime = mtime;
                e.minZ = tv.minZ;
                e.maxZ = tv.maxZ;
                e.keys = tv.keys;
                cache.byRelPath[rel] = std::move(e);
                cacheUpdated = true;
            }
        }));
    }

    for (auto& w : workers) w.get();

    if (useCache && cacheUpdated) {
        if (!saveVoxelCache(cachePath, cache)) {
            std::cerr << "[CACHE] WARNING: no pude escribir cache: " << cachePath.string() << "\n";
        } else {
            std::cerr << "[CACHE] updated file=" << cachePath.string() << "\n";
        }
    } else if (useCache) {
        std::cerr << "[CACHE] hit file=" << cachePath.string() << "\n";
    }

    return trees;
}
