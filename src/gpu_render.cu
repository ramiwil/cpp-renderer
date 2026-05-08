#include "gpu_render.h"
#include "cuda_runtime.h"
#include <curand_kernel.h> 
#include "primitives/object.h"
#include "primitives/plane.h"

__device__ hit_result hit_plane(plane_g p, Ray ray) {
    Vec3 center(p.cx, p.cy, p.cz);
    Vec3 normal(p.nx, p.ny, p.nz);
    Vec3 ray_origin = ray.get_origin();
    Vec3 ray_direction = ray.get_direction();

    float numerator = (center - ray_origin).dot(normal);
    float denom = ray_direction.dot(normal);

    if (denom == 0) return hit_result{0, Vec3(0), Vec3(0), false};

    float t = numerator / denom;
    if (t < 0) return hit_result{0, Vec3(0), Vec3(0), false};

    Vec3 surface_point = ray_origin + ray_direction * t;

    Vec3 ref = std::abs(normal.x) < 0.9f ? Vec3(1, 0, 0) : Vec3(0, 1, 0);
    Vec3 tangent = normal.cross(ref).normalize();
    Vec3 bitangent = normal.cross(tangent);

    Vec3 proj_hit = (surface_point - center);
    float proj_tan = proj_hit.dot(tangent);
    float proj_bitan = proj_hit.dot(bitangent);

    if (std::abs(proj_tan) > p.l / 2 || std::abs(proj_bitan) > p.w / 2)
        return hit_result{0, Vec3(0), Vec3(0), false};

    Vec3 norm = (denom > 0) ? -normal : normal;

    return hit_result{t, surface_point, norm, true};
}

__device__ BxDFSample sample_material(material_g mat, Vec3 normal, Vec3 dir_in, curandState* localState) {
    // Copy state to local register for performance
    float x, y;
    while (true) {
        x = curand_uniform(localState) * 2.0f - 1.0f;
        y = curand_uniform(localState) * 2.0f - 1.0f;
        if (x*x + y*y <= 1.0f) break;
    }
    float z = sqrtf(1.0f - x*x - y*y);

    Vec3 ref = fabsf(normal.x) < 0.9f ? Vec3(1, 0, 0) : Vec3(0, 1, 0);
    Vec3 tangent = normal.cross(ref).normalize();
    Vec3 bitangent = normal.cross(tangent);

    Vec3 dir = (tangent * x + bitangent * y + normal * z).normalize();
    float pdf = normal.dot(dir) / M_PI;

    return {dir, pdf};
}

__device__ Vec3 evaluate_material(material_g mat, Vec3 normal, Vec3 dir_in, Vec3 dir_out, curandState* localState) {
    return mat.albedo / M_PI;
}

__global__ void trace_kernel(float* out, scene_params sp, Camera cam, material_g* mats_d, plane_g* box_d, curandState* state) {
    int col = blockDim.x * blockIdx.x + threadIdx.x;
    int row = blockDim.y * blockIdx.y + threadIdx.y;
    int id = row * (gridDim.x * blockDim.x) + col;

    curand_init(1234, id, 0, &state[id]);
    curandState localState = state[id];

    if ((row < sp.height) && (col < sp.width)) {
        int i = (row * sp.width + col) * 3;
        Vec3 pixel_color(0.0f);
        for (int smp = 0; smp < sp.samples; smp++) {
            float u = (col + curand_uniform(&localState)) / sp.width; 
            float v = (row + curand_uniform(&localState)) / sp.height;
            Ray ray = cam.generate_ray(u, v);
            Vec3 throughput(1.0f);
            for (int d = 0; d < sp.depth; d++) {
                hit_result closest;
                float min_t = INT_MAX;
                int closest_obj = -1;
                for (int pl = 0; pl < 6; pl++) {
                    plane_g p = box_d[pl];
                    auto res = hit_plane(p, ray);
                    if (res.hit && res.t < min_t) {
                        min_t = res.t;
                        closest = res;
                        closest_obj = pl;
                    }
                }

                if (closest_obj == -1) break;
                material_g obj_material = mats_d[box_d[closest_obj].material_id];

                BxDFSample s = sample_material(obj_material, closest.normal, ray.get_negative_direction(), &localState);
                Vec3 f_indirect = evaluate_material(obj_material, closest.normal, s.dir, ray.get_negative_direction(), &localState);
                float cos_theta = closest.normal.dot(s.dir);

                pixel_color += throughput * obj_material.emission * obj_material.emission_strength;

                throughput = throughput * f_indirect * (cos_theta / s.pdf);
                ray = Ray(closest.point + closest.normal * 1e-3, s.dir);
            }
        }

        pixel_color /= sp.samples;
        int i_r = i; int i_g = i + 1; int i_b = i + 2;
        out[i_r] = pixel_color.x; out[i_g] = pixel_color.y; out[i_b] = pixel_color.z;
    }
}


std::vector<plane_g> generate_cornell_box() {
    std::vector<plane_g> ps;
    float size = 100.0f;
    plane_g ceiling = {0, 0.0f, size / 2.0f, 0.0f, 0.0f, 1.0f, 0.0f, size, size};
    ps.emplace_back(ceiling);

    plane_g floor = {0, 0.0f, -size / 2.0f, 0.0f, 0.0f, 1.0f, 0.0f, size, size};
    ps.emplace_back(floor);

    plane_g back_wall = {0, 0.0f, 0.0f, size / 2.0f, 0.0f, 0.0f, 1.0f, size, size};
    ps.emplace_back(back_wall);

    plane_g red_wall = {1, size / 2.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, size, size};
    ps.emplace_back(red_wall);

    plane_g green_wall = {2, -size / 2.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, size, size};
    ps.emplace_back(green_wall);

    plane_g light = {3, 0.0f, 49.9f, 0.0f, 0.0f, 1.0f, 0.0f, 20.0f, 20.0f};
    ps.emplace_back(light);
    
    return ps;
}


float* render_gpu(const Camera& cam, const Scene& scene, scene_params sp) {
    int size = sp.width * sp.height * sizeof(float) * 3;

    float* frame_buffer_h = (float*)malloc(size);
    float* frame_buffer_d;

    // materials
    std::vector<material_g> mats;
    material_g white = {LAMBERTIAN, Vec3(1.0f), Vec3(0.0f), 0.0};
    mats.emplace_back(white);
    material_g red = {LAMBERTIAN, Vec3(1.0f, 0.0f, 0.0f), Vec3(0.0f), 0.0};
    mats.emplace_back(red);
    material_g green = {LAMBERTIAN, Vec3(0.0f, 1.0f, 0.0f), Vec3(0.0f), 0.0};
    mats.emplace_back(green);
    material_g light = {LAMBERTIAN, Vec3(1.0f), Vec3(1.0f), 10.0};
    mats.emplace_back(light);
    int mats_size = sizeof(material_g) * mats.size();
    material_g* mats_d;


    // planes
    auto box = generate_cornell_box();
    int box_size = sizeof(plane_g) * box.size();
    plane_g* box_d;


    cudaMalloc((void**)&frame_buffer_d, size);
    cudaMalloc((void**)&box_d, box_size);
    cudaMalloc((void**)&mats_d, mats_size);

    cudaMemcpy(box_d, box.data(), box_size, cudaMemcpyHostToDevice);
    cudaMemcpy(mats_d, mats.data(), mats_size, cudaMemcpyHostToDevice);

    int n = sp.width * sp.height; // Number of random sequences (one per thread)
    curandState *d_states;
    // Allocate global memory on the GPU to hold the states
    cudaMalloc((void**)&d_states, n * sizeof(curandState));


    int blockSize = 16;
    dim3 blockDim(blockSize, blockSize, 1);
    dim3 gridDim(ceil(sp.width/blockSize), ceil(sp.height/blockSize), 1);

    trace_kernel<<<gridDim, blockDim>>>(
        frame_buffer_d, 
        sp,
        cam,
        mats_d,
        box_d,
        d_states
    );

    cudaMemcpy(frame_buffer_h, frame_buffer_d, size, cudaMemcpyDeviceToHost);
    cudaFree(frame_buffer_d);
    cudaFree(box_d);

    return frame_buffer_h;
}
