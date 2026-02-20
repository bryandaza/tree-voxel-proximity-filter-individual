# Tree Voxel Proximity Filter (Individual Mode)

## Abstract
This project implements an individual (tree-by-tree) 3D proximity screening tool for segmented terrestrial laser scanning (TLS) tree point clouds stored as per-tree PLY files. Each tree is discretized into a 3D voxel occupancy set, and potential contacts are detected by testing whether the target tree shares at least one voxel (or voxel-neighborhood) with any other tree. A persistent binary cache is used to avoid repeated voxelization and accelerate repeated analyses over the same dataset.

## 1. Motivation
In dense forests, segmented trees may appear spatially close due to crown interpenetration, branch-to-branch contact, or segmentation ambiguity. Manual verification is usually required, but full all-pairs comparisons can be computationally expensive and unnecessary when the practical workflow is tree-by-tree validation. This tool supports that workflow: one run evaluates one target tree and produces a compact candidate list for visual inspection.

## 2. Method Summary
Given a directory containing one PLY file per tree, the tool:
1. Reads each tree point cloud and converts it into a set of occupied voxels at resolution `--voxel` (meters).
2. Writes/reads a persistent cache (`voxels_cache.bin` by default) with voxel occupancy per tree file, so repeated runs avoid re-parsing all PLY files.
3. For a target tree (`--tree`), searches for candidate trees that “touch” the target according to:
   - **Exact voxel intersection** if `--dilate 0`
   - **Neighborhood intersection** if `--dilate > 0` (e.g., `d=1` checks a 3×3×3 neighborhood)
4. Creates an output folder named after the target tree, and writes a CSV with overlapping candidates.

## 3. Input Requirements
- **Format:** PLY files with vertex properties `x`, `y`, `z`.
- **Units:** Coordinates are expected to be in meters (consistent with `--voxel`).
- **Layout:** All tree files are located in the directory passed via `--input`.

## 4. Output
Each run creates:
- `results/<TARGET_TREE_NAME>/overlaps.csv`

### 4.1 CSV structure
The output CSV contains one row per candidate tree that touches the target (according to voxel criteria):

- `target_file`: target filename
- `target_id`: target internal index (0-based, as loaded)
- `target_height_m`: target height estimated as `(maxZ - minZ)`
- `other_file`: candidate filename
- `other_id`: candidate internal index (0-based)
- `other_height_m`: candidate height estimated as `(maxZ - minZ)`
- `hits`: number of candidate voxels that found at least one touching voxel (or neighborhood voxel) in the target
- `voxel_m`: voxel size used (meters)
- `dilate`: dilation used

> Interpretation: `hits` is a proximity score. Higher values typically indicate a larger region of contact/proximity, but it is not a geometric distance metric.

## 5. Installation and Compilation

### 5.1 Linux (GCC/Clang)
```bash
cmake -S . -B build
cmake --build build -j
```

### 5.2 Windows (MSVC, PowerShell)
```bash
cmake -S . -B build
cmake --build build --config Release
```

### 6. Usage

# Linux:
```bash
./build/tree_voxel_proximity_individual \
  --input Data/B04_individual_pcs \
  --tree AMA_B04_2cm_5987_159648_segmented_0.ply \
  --voxel 0.02 \
  --dilate 1 \
  --threads 8 \
  --outdir results
```
# Windows
```bash
.\build\Release\tree_voxel_proximity_individual.exe `
  --input "Data\B04_individual_pcs" `
  --tree "AMA_B04_2cm_5987_159648_segmented_0.ply" `
  --voxel 0.02 `
  --dilate 1 `
  --threads 8 `
  --outdir "results"
```