#pragma once
#include <string>
#include <iostream>
#include <cstdlib>
#include <stdexcept>
#include <algorithm>

struct Args {
    std::string inputDir;
    std::string treeFile;              // requerido
    double voxel = 0.02;               // 2 cm default
    int dilate = 1;                    // 0 exacto, 1 vecinos 3x3x3
    int threads = 8;

    bool useCache = true;
    std::string cacheFile = "voxels_cache.bin";

    std::string outRoot = "results";   // carpeta donde se creará <outRoot>/<treeName>/overlaps.csv
};

static inline void printUsage() {
    std::cout <<
    "tree_voxel_proximity_individual\n\n"
    "Uso:\n"
    "  tree_voxel_proximity_individual --input <dir> --tree <archivo.ply>\n"
    "      [--voxel v] [--dilate d] [--threads n]\n"
    "      [--cache <file>] [--no-cache]\n"
    "      [--outdir <dir>]\n\n"
    "Parámetros:\n"
    "  --input <dir>     Directorio con .ply (un archivo por árbol)\n"
    "  --tree <file>     Archivo target (puede ser nombre o ruta)\n"
    "  --voxel <m>       Tamaño de voxel en metros (ej 0.02)\n"
    "  --dilate <k>      Tolerancia en voxels (0=exacto, 1=vecinos)\n"
    "  --threads <n>     Hilos para lectura/voxelización (cache build)\n"
    "  --cache <file>    Archivo cache (default voxels_cache.bin)\n"
    "  --no-cache        Desactiva cache\n"
    "  --outdir <dir>    Carpeta raíz de salida (default results)\n";
}

static inline Args parseArgs(int argc, char** argv) {
    Args a;
    for (int i = 1; i < argc; ++i) {
        std::string k = argv[i];
        auto needValue = [&](const char* opt) {
            if (i + 1 >= argc) throw std::runtime_error(std::string("Falta valor para ") + opt);
            return std::string(argv[++i]);
        };

        if (k == "--input") a.inputDir = needValue("--input");
        else if (k == "--tree") a.treeFile = needValue("--tree");
        else if (k == "--voxel") a.voxel = std::stod(needValue("--voxel"));
        else if (k == "--dilate") a.dilate = std::stoi(needValue("--dilate"));
        else if (k == "--threads") a.threads = std::stoi(needValue("--threads"));
        else if (k == "--cache") a.cacheFile = needValue("--cache");
        else if (k == "--no-cache") a.useCache = false;
        else if (k == "--outdir") a.outRoot = needValue("--outdir");
        else if (k == "--help" || k == "-h") { printUsage(); std::exit(0); }
        else throw std::runtime_error("Argumento desconocido: " + k);
    }

    if (a.inputDir.empty()) { printUsage(); throw std::runtime_error("Debes pasar --input <dir>"); }
    if (a.treeFile.empty()) { printUsage(); throw std::runtime_error("Debes pasar --tree <archivo.ply>"); }
    if (a.voxel <= 0.0) throw std::runtime_error("--voxel debe ser > 0");
    if (a.dilate < 0 || a.dilate > 3) throw std::runtime_error("--dilate fuera de rango (0..3)");
    if (a.threads <= 0) a.threads = 1;

    return a;
}
