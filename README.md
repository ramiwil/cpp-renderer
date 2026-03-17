# Path Tracer

A physically-based path tracer built from scratch in C++ to learn rendering theory and build a foundation for advanced path tracing techniques and graphics research.

![Cornell box](renders/output.png)


## Overview

An unidirectional Monte Carlo path tracer implementing the rendering equation. The goal of this project is to build a solid foundational knowledge before moving towards advanced path tracing techniques and graphics research.


## Features

- Monte Carlo path tracing
- Lambertian BRDF
    - Cosine-weighted hemisphere sampling
- Multi-sample anti-aliasing
- Multi-threaded rendering


## Build and run

```
cmake -Bbuild -GNinja
cmake --build build -t run
```

Requires a C++17 compiler and OpenMP. Resolution and sample count are configured in `src/main.cpp`.


## Gallery

TBD


## Roadmap

- ~~Cosine-weighted hemisphere sampling (Lambertian BRDF)~~ ✓
- Mirror reflection and metallic materials
- Glass / dielectrics
- Multiple importance sampling (MIS)
- GGX microfacet BRDF
- Bounding volume hierarchy (BVH) for sublinear ray-object intersection
- Triangle mesh loading (OBJ)
- HDR environment maps / image-based lighting
- Volumetric rendering / participating media
- Subsurface scattering
- Caustics
