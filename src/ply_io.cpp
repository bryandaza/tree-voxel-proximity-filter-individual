#include "ply_io.h"
#include "tinyply.h"
#include <fstream>
#include <cmath>
#include <limits>
#include <algorithm>

using namespace tinyply;
namespace fs = std::filesystem;

static constexpr int32_t BIAS = 1 << 20;

static inline uint64_t packIJK(int32_t ix, int32_t iy, int32_t iz) {
    const uint64_t x = (uint32_t)(ix + BIAS) & 0x1FFFFF;
    const uint64_t y = (uint32_t)(iy + BIAS) & 0x1FFFFF;
    const uint64_t z = (uint32_t)(iz + BIAS) & 0x1FFFFF;
    return (x << 42) | (y << 21) | z;
}

static inline double readAt(const std::shared_ptr<PlyData>& d, size_t i) {
    if (d->t == Type::FLOAT32) return (double)reinterpret_cast<const float*>(d->buffer.get())[i];
    if (d->t == Type::FLOAT64) return reinterpret_cast<const double*>(d->buffer.get())[i];
    throw std::runtime_error("Tipo de vertex no soportado (esperaba float32/float64)");
}

static inline int32_t qIndex(double v, double voxelSize) {
    return (int32_t)std::floor(v / voxelSize);
}

TreeVoxels loadAndVoxelizePLY(const fs::path& file, double voxelSize) {
    TreeVoxels tv;
    tv.file = file.string();

    std::ifstream ss(file, std::ios::binary);
    if (!ss) throw std::runtime_error("No pude abrir: " + file.string());

    PlyFile ply;
    ply.parse_header(ss);

    std::shared_ptr<PlyData> vx, vy, vz;
    vx = ply.request_properties_from_element("vertex", { "x" });
    vy = ply.request_properties_from_element("vertex", { "y" });
    vz = ply.request_properties_from_element("vertex", { "z" });

    ply.read(ss);

    const size_t n = vx->count;
    if (n == 0) throw std::runtime_error("PLY sin puntos: " + file.string());
    if (vy->count != n || vz->count != n) throw std::runtime_error("Counts x/y/z inconsistentes: " + file.string());

    tv.keys.reserve(n / 10);

    double minZ = std::numeric_limits<double>::infinity();
    double maxZ = -std::numeric_limits<double>::infinity();

    for (size_t i = 0; i < n; ++i) {
        const double x = readAt(vx, i);
        const double y = readAt(vy, i);
        const double z = readAt(vz, i);

        minZ = std::min(minZ, z);
        maxZ = std::max(maxZ, z);

        const int32_t ix = qIndex(x, voxelSize);
        const int32_t iy = qIndex(y, voxelSize);
        const int32_t iz = qIndex(z, voxelSize);

        tv.keys.push_back(packIJK(ix, iy, iz));
    }

    std::sort(tv.keys.begin(), tv.keys.end());
    tv.keys.erase(std::unique(tv.keys.begin(), tv.keys.end()), tv.keys.end());

    tv.minZ = minZ;
    tv.maxZ = maxZ;
    return tv;
}

