# ---------- build stage ----------
FROM ubuntu:24.04 AS builder

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential cmake ninja-build \
 && rm -rf /var/lib/apt/lists/*

WORKDIR /repo
COPY . .

# Compilar en Release (Linux)
RUN cmake -S . -B build_docker -G Ninja -DCMAKE_BUILD_TYPE=Release \
 && cmake --build build_docker

# ---------- runtime stage ----------
FROM ubuntu:24.04 AS runtime

# Runtime mínimo típico para binarios C++ en Ubuntu
RUN apt-get update && apt-get install -y --no-install-recommends \
    libstdc++6 \
 && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copia el ejecutable
COPY --from=builder /repo/build_docker/tree_voxel_proximity_individual /app/tree_voxel_proximity_individual

# (Opcional) crea dirs esperadas
RUN mkdir -p /app/Data /app/results

ENTRYPOINT ["/app/tree_voxel_proximity_individual"]