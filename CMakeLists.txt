cmake_minimum_required(VERSION 3.20)
project(tree_voxel_proximity_individual CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(tree_voxel_proximity_individual
  src/main.cpp
  src/ply_io.cpp
  src/voxel_cache.cpp
  src/build_trees_cache.cpp
  src/individual_overlap.cpp
  src/csv_writer.cpp
  external/tinyply.cpp
)

target_include_directories(tree_voxel_proximity_individual PRIVATE
  src
  external
)

if (MSVC)
  target_compile_options(tree_voxel_proximity_individual PRIVATE /W4 /O2)
else()
  target_compile_options(tree_voxel_proximity_individual PRIVATE -O3 -Wall -Wextra -Wpedantic)
endif()
