#include "voxel_cache.h"
#include <fstream>
#include <chrono>

namespace fs = std::filesystem;

static void writeU32(std::ostream& os, uint32_t v){ os.write((char*)&v, sizeof(v)); }
static void writeU64(std::ostream& os, uint64_t v){ os.write((char*)&v, sizeof(v)); }
static void writeI64(std::ostream& os, int64_t v){ os.write((char*)&v, sizeof(v)); }
static void writeF64(std::ostream& os, double v){ os.write((char*)&v, sizeof(v)); }

static bool readU32(std::istream& is, uint32_t& v){ return (bool)is.read((char*)&v, sizeof(v)); }
static bool readU64(std::istream& is, uint64_t& v){ return (bool)is.read((char*)&v, sizeof(v)); }
static bool readI64(std::istream& is, int64_t& v){ return (bool)is.read((char*)&v, sizeof(v)); }
static bool readF64(std::istream& is, double& v){ return (bool)is.read((char*)&v, sizeof(v)); }

static void writeString(std::ostream& os, const std::string& s){
    writeU32(os, (uint32_t)s.size());
    os.write(s.data(), (std::streamsize)s.size());
}
static bool readString(std::istream& is, std::string& s){
    uint32_t n=0; if(!readU32(is,n)) return false;
    s.resize(n);
    return (bool)is.read(&s[0], (std::streamsize)n);
}

uint64_t getFileSizeU64(const fs::path& p){ return (uint64_t)fs::file_size(p); }

int64_t getFileMTimeI64(const fs::path& p){
    auto ftime = fs::last_write_time(p);
    auto s = std::chrono::time_point_cast<std::chrono::seconds>(ftime).time_since_epoch().count();
    return (int64_t)s;
}

std::string makeRelPath(const fs::path& inputDir, const fs::path& file){
    auto rel = fs::relative(file, inputDir);
    return rel.generic_string();
}

bool saveVoxelCache(const fs::path& cacheFile, const VoxelCacheData& cache){
    std::ofstream out(cacheFile, std::ios::binary);
    if(!out) return false;

    out.write("VOXCACHE", 8);
    writeU32(out, cache.version);
    writeF64(out, cache.voxelSize);

    writeU32(out, (uint32_t)cache.byRelPath.size());
    for(const auto& kv : cache.byRelPath){
        const CachedTree& e = kv.second;
        writeString(out, e.relPath);
        writeU64(out, e.fileSize);
        writeI64(out, e.mtime);
        writeF64(out, e.minZ);
        writeF64(out, e.maxZ);

        writeU32(out, (uint32_t)e.keys.size());
        out.write((const char*)e.keys.data(), (std::streamsize)e.keys.size() * sizeof(uint64_t));
    }
    return true;
}

bool loadVoxelCache(const fs::path& cacheFile, VoxelCacheData& outCache){
    std::ifstream in(cacheFile, std::ios::binary);
    if(!in) return false;

    char magic[8];
    if(!in.read(magic, 8)) return false;
    if(std::string(magic, 8) != "VOXCACHE") return false;

    uint32_t ver=0; if(!readU32(in, ver)) return false;
    double voxelSize=0; if(!readF64(in, voxelSize)) return false;

    uint32_t n=0; if(!readU32(in, n)) return false;

    outCache = {};
    outCache.version = ver;
    outCache.voxelSize = voxelSize;
    outCache.byRelPath.reserve(n * 2);

    for(uint32_t i=0;i<n;++i){
        CachedTree e;
        if(!readString(in, e.relPath)) return false;
        if(!readU64(in, e.fileSize)) return false;
        if(!readI64(in, e.mtime)) return false;
        if(!readF64(in, e.minZ)) return false;
        if(!readF64(in, e.maxZ)) return false;

        uint32_t k=0; if(!readU32(in, k)) return false;
        e.keys.resize(k);
        if(!in.read((char*)e.keys.data(), (std::streamsize)k * sizeof(uint64_t))) return false;

        outCache.byRelPath[e.relPath] = std::move(e);
    }
    return true;
}
