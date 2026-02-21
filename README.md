# Tree Voxel Proximity Filter (Individual Mode)

## Abstract
This project implements an individual (tree-by-tree) 3D proximity screening tool for segmented terrestrial laser scanning (TLS) tree point clouds stored as per-tree PLY files. Each tree is discretized into a 3D voxel occupancy set, and potential contacts are detected by testing whether the target tree shares at least one voxel (or voxel-neighborhood) with any other tree. A persistent binary cache is used to avoid repeated voxelization and accelerate repeated analyses over the same dataset.

This repository provides a Dockerized build to ensure reproducible execution across platforms.

## Motivation
In dense forests, segmented trees may appear spatially close due to crown interpenetration, branch-to-branch contact, or segmentation ambiguity. Manual verification is usually required, but full all-pairs comparisons can be computationally expensive and unnecessary when the practical workflow is tree-by-tree validation. This tool supports that workflow: one run evaluates one target tree and produces a compact candidate list for visual inspection.

## Method Summary
Given a directory containing one PLY file per tree, the tool:
1. Reads each tree point cloud and converts it into a set of occupied voxels at resolution `--voxel` (meters).
2. Writes/reads a persistent cache (`voxels_cache.bin` by default) with voxel occupancy per tree file, so repeated runs avoid re-parsing all PLY files.
3. For a target tree (`--tree`), searches for candidate trees that “touch” the target according to:
   - **Exact voxel intersection** if `--dilate 0`
   - **Neighborhood intersection** if `--dilate > 0` (e.g., `d=1` checks a 3×3×3 neighborhood)
4. Creates an output folder named after the target tree, and writes a CSV with overlapping candidates.

## Requirements

### Common
- Git
- Docker Engine (Linux) or Docker Desktop (Windows/macOS)
- Docker running in **Linux containers mode** (Windows users)

### Input
- **Format:** PLY files with vertex properties `x`, `y`, `z`.
- **Units:** Coordinates are expected to be in meters (consistent with `--voxel`).
- **Layout:** All tree files are located in the directory passed via `--input`.

### Output
Each run creates:
- `results/<TARGET_TREE_NAME>/overlaps.csv`

### CSV structure
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

## Installation and Compilation (Local – via Git Clone)
### Windows (PowerShell)
```powersshell
git clone https://github.com/bryandaza/tree-voxel-proximity-filter-individual.git
cd tree-voxel-proximity-filter-individual
```
# Linux / macOS (bash):
```bash
git clone https://github.com/bryandaza/tree-voxel-proximity-filter-individual.git
cd tree-voxel-proximity-filter-individual
```
# Usage
### Windows (PowerShell)
```bash
docker run --rm `
  -v "${PWD}\Data:/app/Data" `
  -v "${PWD}\results:/app/results" `
  tree-voxel-individual `
  --input "/app/Data/B04_individual_pcs" `
  --tree "AMA_B04_2cm_5987_159648_segmented_0.ply" `
  --voxel 0.02 `
  --dilate 1 `
  --threads 8 `
  --outdir "/app/results"
```
# Linux / macOS (bash):
```bash
docker run --rm \
  -v "$(pwd)/Data:/app/Data" \
  -v "$(pwd)/results:/app/results" \
  tree-voxel-individual \
  --input "/app/Data/B04_individual_pcs" \
  --tree "AMA_B04_2cm_5987_159648_segmented_0.ply" \
  --voxel 0.02 \
  --dilate 1 \
  --threads 8 \
  --outdir "/app/results"
```
Important: why `-v` is required

The tool reads input files from your local `Data/` folder and writes outputs to your local `results/` folder.

Docker containers are isolated by default, so we mount these folders using `-v:`

`./Data` -> `/app/Data` (inside container)
`./results` -> `/app/results` (inside container)
