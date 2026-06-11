#include "cuda_runtime.h"
#include "gpu_render.h"
#include "primitives/object.h"
#include "primitives/plane.h"
#include <curand_kernel.h>

__device__ float schlick_reflectance(float cos, float ri) {
	auto r0 = (ri - 1.0f) / (ri + 1.0f);
	r0 = r0 * r0;
	return r0 + (1 - r0) * powf(1 - cos, 5);
}
__device__ hit_result hit_plane(plane_g p, Ray ray) {
	Vec3 center(p.cx, p.cy, p.cz);
	Vec3 normal(p.nx, p.ny, p.nz);
	Vec3 ray_origin = ray.get_origin();
	Vec3 ray_direction = ray.get_direction();

	float numerator = (center - ray_origin).dot(normal);
	float denom = ray_direction.dot(normal);

	if (denom == 0)
		return hit_result{0, Vec3(0), Vec3(0), false};

	float t = numerator / denom;
	if (t < 0)
		return hit_result{0, Vec3(0), Vec3(0), false};

	Vec3 surface_point = ray_origin + ray_direction * t;

	Vec3 ref = fabsf(normal.x) < 0.9f ? Vec3(1, 0, 0) : Vec3(0, 1, 0);
	Vec3 tangent = normal.cross(ref).normalize();
	Vec3 bitangent = normal.cross(tangent);

	Vec3 proj_hit = (surface_point - center);
	float proj_tan = proj_hit.dot(tangent);
	float proj_bitan = proj_hit.dot(bitangent);

	if (fabsf(proj_tan) > p.l / 2 || fabsf(proj_bitan) > p.w / 2)
		return hit_result{0, Vec3(0), Vec3(0), false};

	Vec3 norm = (denom > 0) ? -normal : normal;

	return hit_result{t, surface_point, norm, true};
}

__device__ hit_result hit_sphere(sphere_g s, Ray ray) {
	Vec3 center(s.cx, s.cy, s.cz);
	Vec3 ray_origin = ray.get_origin();
	Vec3 ray_direction = ray.get_direction();
	float radius = s.radius;

	Vec3 l = center - ray_origin;
	float tca = l.dot(ray_direction);
	if (tca < 0.0) {
		return hit_result{0, Vec3(0), Vec3(0), false};
	}

	float d2 = l.dot(l) - tca * tca;
	if (d2 > radius * radius)
		return hit_result{0, Vec3(0), Vec3(0), false};
	float thc = sqrtf(radius * radius - d2);
	float t0 = tca - thc;
	float t1 = tca + thc;

	float t = (t0 > 0) ? t0 : t1;
	if (t < 0)
		return hit_result{0, Vec3(0), Vec3(0), false};

	Vec3 surface_point = ray_origin + ray_direction * t;
	Vec3 surface_normal = (surface_point - center).normalize();

	return hit_result{t, surface_point, surface_normal, true};
}

__device__ BxDFSample sample_material(material_g mat, Vec3 normal, Vec3 dir_in,
									  curandState *localState) {
	if (mat.type == LAMBERTIAN) {
		float x, y;
		while (true) {
			x = curand_uniform(localState) * 2.0f - 1.0f;
			y = curand_uniform(localState) * 2.0f - 1.0f;
			if (x * x + y * y <= 1.0f)
				break;
		}
		float z = sqrtf(1.0f - x * x - y * y);

		Vec3 ref = fabsf(normal.x) < 0.9f ? Vec3(1, 0, 0) : Vec3(0, 1, 0);
		Vec3 tangent = normal.cross(ref).normalize();
		Vec3 bitangent = normal.cross(tangent);

		Vec3 dir = (tangent * x + bitangent * y + normal * z).normalize();
		float pdf = normal.dot(dir) / M_PI;

		return {dir, pdf};
	} else if (mat.type == METAL) {
		Vec3 reflection = 2 * (dir_in.dot(normal)) * normal - dir_in;

		return {reflection.normalize(), 1.0f};
	} else if (mat.type == GLASS) {
		bool entering = dir_in.dot(normal) > 0.0f;
		float eta = entering ? (1.0f / mat.ior) : mat.ior;
		Vec3 n = entering ? normal : -normal;

		float cos_theta = dir_in.dot(n);

		bool cannot_refract = (eta * sqrtf(1.0f - cos_theta * cos_theta)) > 1.0f;
		Vec3 direction;
		if (cannot_refract || schlick_reflectance(cos_theta, eta) > curand_uniform(localState)) {
			direction = (2 * (dir_in.dot(normal)) * normal - dir_in).normalize();
		} else {
			Vec3 perp = eta * (-dir_in + cos_theta * n);
			float discriminant = 1.0f - (perp.length() * perp.length());
			if (discriminant < 0.0f) {
				Vec3 reflection = (2 * cos_theta * n - dir_in).normalize();
				return {reflection, 1.0f};
			}
			Vec3 parallel = -sqrtf(discriminant) * n;
			Vec3 refract = (perp + parallel).normalize();
			direction = refract;
		}
		return {direction, 1.0f};
	}

	return {Vec3(1.0f), 1.0f};
}

__device__ Vec3 sample_light(light_g light, curandState *localState) {
	Vec3 position(light.cx, light.cy, light.cz);
	Vec3 normal(light.nx, light.ny, light.nz);
	Vec3 ref = fabsf(normal.x) < 0.9f ? Vec3(1, 0, 0) : Vec3(0, 1, 0);
	Vec3 tangent = normal.cross(ref).normalize();
	Vec3 bitangent = normal.cross(tangent);

	float x_off = (curand_uniform(localState) * 2 - 1) * light.l / 2.0f;
	float y_off = (curand_uniform(localState) * 2 - 1) * light.w / 2.0f;
	Vec3 point = position + tangent * x_off + bitangent * y_off;

	return point;
}

__device__ Vec3 evaluate_material(material_g mat, Vec3 normal, Vec3 dir_in, Vec3 dir_out,
								  curandState *localState) {
	if (mat.type == LAMBERTIAN) {
		return mat.albedo / M_PI;
	} else if (mat.type == METAL) {
		return mat.albedo;
	} else if (mat.type == GLASS) {
		return mat.albedo;
	}
	return Vec3(1.0f);
}

__global__ void trace_kernel(float *out, scene_params sp, Camera cam, curandState *state) {
	int col = blockDim.x * blockIdx.x + threadIdx.x;
	int row = blockDim.y * blockIdx.y + threadIdx.y;
	int id = row * (gridDim.x * blockDim.x) + col;

	curandState localState = state[id];

	if ((row < sp.height) && (col < sp.width)) {
		int i = (row * sp.width + col) * 3;
		Vec3 pixel_color(0.0f);
		for (int smp = 0; smp < sp.samples; smp++) {
			float u = (col + curand_uniform(&localState)) / sp.width;
			float v = (row + curand_uniform(&localState)) / sp.height;
			Ray ray = cam.generate_ray(u, v);
			Vec3 throughput(1.0f);
			bool prev_was_specular = false;
			for (int d = 0; d < sp.depth; d++) {
				hit_result closest;
				float min_t = INT_MAX;
				int closest_obj = -1;
				int closest_material_id = -1;
				for (int pl = 0; pl < sp.num_planes; pl++) {
					plane_g p = sp.planes[pl];
					auto res = hit_plane(p, ray);
					if (res.hit && res.t < min_t) {
						min_t = res.t;
						closest = res;
						closest_obj = pl;
						closest_material_id = p.material_id;
					}
				}
				for (int ball = 0; ball < sp.num_spheres; ball++) {
					sphere_g b = sp.spheres[ball];
					auto res = hit_sphere(b, ray);
					if (res.hit && res.t < min_t) {
						min_t = res.t;
						closest = res;
						closest_obj = ball;
						closest_material_id = b.material_id;
					}
				}

				for (int ligh = 0; ligh < sp.num_lights; ligh++) {
					light_g l = sp.lights[ligh];
					plane_g ligh_p = {l.material_id, l.cx, l.cy, l.cz, l.nx, l.ny, l.nz, l.w, l.l};
					auto res = hit_plane(ligh_p, ray);
					if (res.hit && res.t < min_t) {
						min_t = res.t;
						closest = res;
						closest_obj = ligh;
						closest_material_id = l.material_id;
					}
				}

				if (closest_obj == -1)
					break;
				material_g obj_material = sp.materials[closest_material_id];

				float EPS = 1e-3;

				// direct sampling (next event estimation)
				Vec3 f_direct(0.0f);
				if (obj_material.type != GLASS && obj_material.type != METAL) {
					for (int light = 0; light < sp.num_lights; light++) {
						auto l = sp.lights[light];
						Vec3 light_point = sample_light(l, &localState);

						Vec3 light_normal(l.nx, l.ny, l.nz);
						Vec3 to_light = light_point - closest.point;
						float dist_sq = to_light.dot(to_light);
						float dist = sqrtf(dist_sq);
						Vec3 dir_to_light = (to_light / dist).normalize();

						Ray shadow_ray(closest.point + closest.normal * EPS, dir_to_light);
						bool occluded_s = false;
						for (int pl = 0; pl < sp.num_planes; pl++) {
							plane_g p = sp.planes[pl];
							auto res = hit_plane(p, shadow_ray);
							if (res.hit && res.t < dist - 2 * EPS) {
								occluded_s = true;
								break;
							}
						}
						bool occ_b = false;
						for (int ball = 0; ball < sp.num_spheres; ball++) {
							sphere_g b = sp.spheres[ball];
							auto res = hit_sphere(b, shadow_ray);
							if (res.hit && res.t < dist - 2 * EPS) {
								occ_b = true;
								break;
							}
						}

						if (!(occluded_s || occ_b)) {
							float cos_surface = closest.normal.dot(dir_to_light);
							float cos_light = light_normal.dot(dir_to_light);
							if (cos_surface > 0 && cos_light > 0) {
								material_g light_mat = sp.materials[l.material_id];
								Vec3 Le = light_mat.emission * light_mat.emission_strength;
								Vec3 f =
									evaluate_material(obj_material, closest.normal, dir_to_light,
													  ray.get_negative_direction(), &localState);
								f_direct += f * Le * cos_surface * cos_light /
											(dist_sq * (1.0f / (l.l * l.w)));
							}
						}
					}
				}

				// indirect sampling
				BxDFSample s = sample_material(obj_material, closest.normal,
											   ray.get_negative_direction(), &localState);
				Vec3 f_indirect = evaluate_material(obj_material, closest.normal, s.dir,
													ray.get_negative_direction(), &localState);
				float cos_theta = closest.normal.dot(s.dir);

				if (d == 0 || prev_was_specular) {
					pixel_color +=
						throughput *
						(obj_material.emission * obj_material.emission_strength + f_direct);
				} else {
					pixel_color += throughput * f_direct;
				}

				if (obj_material.type == GLASS) {
					throughput = throughput * f_indirect;
					ray = Ray(closest.point + s.dir * EPS, s.dir);
				} else {
					throughput = throughput * f_indirect * (cos_theta / s.pdf);
					ray = Ray(closest.point + closest.normal * 1e-3, s.dir);
				}

				prev_was_specular = (obj_material.type == GLASS || obj_material.type == METAL);
			}
		}

		pixel_color /= sp.samples;
		int i_r = i;
		int i_g = i + 1;
		int i_b = i + 2;
		out[i_r] = pixel_color.x;
		out[i_g] = pixel_color.y;
		out[i_b] = pixel_color.z;
		state[id] = localState;
	}
}

__global__ void init_rng(curandState *state, int width) {
	int col = blockDim.x * blockIdx.x + threadIdx.x;
	int row = blockDim.y * blockIdx.y + threadIdx.y;
	int id = row * (gridDim.x * blockDim.x) + col;
	curand_init(width, id, 0, &state[id]);
}

__global__ void accumulate_samples_kernel(float *d_output, float *d_acc, int num_frames,
										  scene_params sp) {
	int col = blockDim.x * blockIdx.x + threadIdx.x;
	int row = blockDim.y * blockIdx.y + threadIdx.y;

	if ((row < sp.height) && (col < sp.width)) {
		int i = (row * sp.width + col) * 3;

		d_acc[i] += d_output[i];
		d_acc[i + 1] += d_output[i + 1];
		d_acc[i + 2] += d_output[i + 2];

		d_output[i] = d_acc[i] / num_frames;
		d_output[i + 1] = d_acc[i + 1] / num_frames;
		d_output[i + 2] = d_acc[i + 2] / num_frames;
	}
}

void *dstate_rng_init(int blockSize, int width, int height) {
	curandState *d_states;
	cudaMalloc((void **)&d_states, (width * height) * sizeof(curandState));
	dim3 block(blockSize, blockSize, 1);
	dim3 grid(ceil(width / blockSize), ceil(height / blockSize), 1);
	init_rng<<<grid, block>>>(d_states, width);

	return (void *)d_states;
}

void *allocate_d_mem(void *s_h, int size) {
	void *p_d;
	cudaMalloc((void **)&p_d, size);
	cudaMemcpy(p_d, s_h, size, cudaMemcpyHostToDevice);
	return p_d;
}

void free_d_mem(void *p_d) { cudaFree(p_d); }

void render_gpu_interop(float *d_output, float *d_acc, int num_frames, void *d_states,
						const Camera &cam, scene_params sp) {
	int blockSize = 16;
	dim3 block(blockSize, blockSize, 1);
	dim3 grid(ceil(sp.width / blockSize), ceil(sp.height / blockSize), 1);

	trace_kernel<<<grid, block>>>(d_output, sp, cam, (curandState *)d_states);
	accumulate_samples_kernel<<<grid, block>>>(d_output, d_acc, num_frames, sp);

	cudaDeviceSynchronize();
}

float *render_gpu(const Camera &cam, scene_params sp) {
	int size = sp.width * sp.height * sizeof(float) * 3;

	// output buffer
	float *frame_buffer_h = (float *)malloc(size);
	float *frame_buffer_d;
	int materials_size = sizeof(material_g) * sp.num_materials;
	int planes_size = sizeof(plane_g) * sp.num_planes;
	int sphere_size = sizeof(sphere_g) * sp.num_spheres;
	material_g *mats_d;
	plane_g *box_d;
	sphere_g *ball_d;

	cudaMalloc((void **)&frame_buffer_d, size);
	cudaMalloc((void **)&mats_d, materials_size);
	cudaMalloc((void **)&box_d, planes_size);
	cudaMalloc((void **)&ball_d, sphere_size);

	cudaMemcpy(mats_d, sp.materials, materials_size, cudaMemcpyHostToDevice);
	cudaMemcpy(box_d, sp.planes, planes_size, cudaMemcpyHostToDevice);
	cudaMemcpy(ball_d, sp.spheres, sphere_size, cudaMemcpyHostToDevice);

	sp.materials = mats_d;
	sp.planes = box_d;
	sp.spheres = ball_d;

	int n = sp.width * sp.height; // Number of random sequences (one per thread)
	curandState *d_states;		  // Allocate global memory on the GPU to hold the states
	cudaMalloc((void **)&d_states, n * sizeof(curandState));

	int blockSize = 16;
	dim3 blockDim(blockSize, blockSize, 1);
	dim3 gridDim(ceil(sp.width / blockSize), ceil(sp.height / blockSize), 1);

	init_rng<<<gridDim, blockDim>>>(d_states, sp.width);

	trace_kernel<<<gridDim, blockDim>>>(frame_buffer_d, sp, cam, d_states);

	cudaMemcpy(frame_buffer_h, frame_buffer_d, size, cudaMemcpyDeviceToHost);
	cudaFree(frame_buffer_d);
	cudaFree(box_d);
	cudaFree(ball_d);
	cudaFree(mats_d);

	return frame_buffer_h;
}
