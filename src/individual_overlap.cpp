#include "individual_overlap.h"
#include <unordered_set>
#include <algorithm>

static constexpr int32_t BIAS = 1 << 20;

static inline uint64_t packIJK(int32_t ix, int32_t iy, int32_t iz) {
    const uint64_t x = (uint32_t)(ix + BIAS) & 0x1FFFFF;
    const uint64_t y = (uint32_t)(iy + BIAS) & 0x1FFFFF;
    const uint64_t z = (uint32_t)(iz + BIAS) & 0x1FFFFF;
    return (x << 42) | (y << 21) | z;
}

static inline void unpackIJK(uint64_t key, int32_t& ix, int32_t& iy, int32_t& iz) {
    const uint32_t x = (uint32_t)((key >> 42) & 0x1FFFFF);
    const uint32_t y = (uint32_t)((key >> 21) & 0x1FFFFF);
    const uint32_t z = (uint32_t)( key        & 0x1FFFFF);
    ix = (int32_t)x - BIAS;
    iy = (int32_t)y - BIAS;
    iz = (int32_t)z - BIAS;
}

std::vector<HitRow> findTouchingForOne(const std::vector<TreeVoxels>& trees,
                                       int targetIdx,
                                       int dilate)
{
    const auto& A = trees[targetIdx];

    std::unordered_set<uint64_t> setA;
    setA.reserve(A.keys.size() * 2);
    for (uint64_t v : A.keys) setA.insert(v);

    std::vector<HitRow> out;
    out.reserve(256);

    for (int j = 0; j < (int)trees.size(); ++j) {
        if (j == targetIdx) continue;
        const auto& B = trees[j];

        uint32_t hits = 0;

        if (dilate == 0) {
            for (uint64_t vb : B.keys) {
                if (setA.find(vb) != setA.end()) hits++;
            }
        } else {
            for (uint64_t vb : B.keys) {
                int32_t ix, iy, iz;
                unpackIJK(vb, ix, iy, iz);

                bool touched = false;
                for (int dx=-dilate; dx<=dilate && !touched; ++dx)
                    for (int dy=-dilate; dy<=dilate && !touched; ++dy)
                        for (int dz=-dilate; dz<=dilate && !touched; ++dz) {
                            uint64_t nb = packIJK(ix+dx, iy+dy, iz+dz);
                            if (setA.find(nb) != setA.end()) {
                                hits++;
                                touched = true; // 1 por voxel de B que logra tocar
                            }
                        }
            }
        }

        if (hits > 0) out.push_back(HitRow{targetIdx, j, hits});
    }

    std::sort(out.begin(), out.end(), [](const HitRow& a, const HitRow& b){
        return a.hits > b.hits;
    });

    return out;
}
