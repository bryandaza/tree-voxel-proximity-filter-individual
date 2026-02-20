#include <iostream>
#include <filesystem>
#include <vector>
#include <algorithm>

#include "args.h"
#include "build_trees_cache.h"
#include "individual_overlap.h"
#include "csv_writer.h"

namespace fs = std::filesystem;

static std::vector<fs::path> listPlyFiles(const fs::path& dir) {
    std::vector<fs::path> out;
    for (const auto& e : fs::directory_iterator(dir)) {
        if (!e.is_regular_file()) continue;
        if (e.path().extension() == ".ply") out.push_back(e.path());
    }
    std::sort(out.begin(), out.end());
    return out;
}

// match por nombre exacto o substring en ruta
static int findTargetIndex(const std::vector<TreeVoxels>& trees, const std::string& treeArg) {
    const std::string argName = fs::path(treeArg).filename().string();

    for (int i = 0; i < (int)trees.size(); ++i) {
        const std::string name = fs::path(trees[i].file).filename().string();
        if (name == argName) return i;
    }
    for (int i = 0; i < (int)trees.size(); ++i) {
        if (trees[i].file.find(treeArg) != std::string::npos) return i;
    }
    return -1;
}

static std::string safeFolderNameFromTree(const std::string& treeFile) {
    std::string s = fs::path(treeFile).stem().string();
    // reemplaza caracteres raros por '_'
    for (char& c : s) {
        if (!(std::isalnum((unsigned char)c) || c=='_' || c=='-' )) c = '_';
    }
    return s;
}

int main(int argc, char** argv) {
    try {
        Args args = parseArgs(argc, argv);

        const fs::path inputDir = fs::path(args.inputDir);
        auto files = listPlyFiles(inputDir);
        if (files.empty()) throw std::runtime_error("No encontré .ply en " + inputDir.string());

        bool cacheUpdated = false;

        // 1) cargar/voxelizar (usando cache)
        auto trees = buildTreesWithCache(
            inputDir, files, args.threads,
            args.cacheFile, args.useCache, args.voxel,
            cacheUpdated
        );

        // 2) encontrar target
        const int targetIdx = findTargetIndex(trees, args.treeFile);
        if (targetIdx < 0) throw std::runtime_error("No encontré el árbol target: " + args.treeFile);

        // 3) buscar solapes SOLO para target
        auto hits = findTouchingForOne(trees, targetIdx, args.dilate);

        // 4) crear carpeta por árbol y escribir CSV ahí
        const std::string folderName = safeFolderNameFromTree(fs::path(trees[targetIdx].file).filename().string());
        fs::path outDir = fs::path(args.outRoot) / folderName;
        fs::path outCsv = outDir / "overlaps.csv";

        if (!writeIndividualCSV(outCsv, trees, hits, targetIdx, args.voxel, args.dilate)) {
            throw std::runtime_error("No pude escribir CSV: " + outCsv.string());
        }

        std::cout << "Target: " << fs::path(trees[targetIdx].file).filename().string()
                  << " (idx=" << targetIdx << ")\n";
        std::cout << "Voxels=" << args.voxel << "  dilate=" << args.dilate
                  << "  threads=" << args.threads << "\n";
        std::cout << "Solapados: " << hits.size() << "\n";
        std::cout << "CSV: " << outCsv.string() << "\n";

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "ERROR: " << e.what() << "\n";
        return 1;
    }
}
