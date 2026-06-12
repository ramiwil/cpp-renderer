#include "cuda_runtime_api.h"
#include "gpu_render.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cstdio>
#include <cuda_gl_interop.h>
#include <iostream>

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

	return ps;
}

std::vector<sphere_g> generate_balls() {
	std::vector<sphere_g> ps;
	sphere_g s1 = {6, 0.0f, -35.0f, 0.0f, 15.0f};
	ps.emplace_back(s1);

	sphere_g s2 = {5, 25.0f, -35.0f, -25.0f, 15.0f};
	ps.emplace_back(s2);

	sphere_g s3 = {4, -25.0f, -35.0f, 25.0f, 15.0f};
	ps.emplace_back(s3);

	return ps;
}

std::vector<material_g> generate_materials() {
	std::vector<material_g> mats;
	material_g white = {LAMBERTIAN, Vec3(1.0f), Vec3(0.0f), 0.0f};
	mats.emplace_back(white);
	material_g red = {LAMBERTIAN, Vec3(1.0f, 0.0f, 0.0f), Vec3(0.0f), 0.0f};
	mats.emplace_back(red);
	material_g green = {LAMBERTIAN, Vec3(0.0f, 1.0f, 0.0f), Vec3(0.0f), 0.0f};
	mats.emplace_back(green);
	material_g light = {LAMBERTIAN, Vec3(1.0f), Vec3(1.0f, 1.0f, 1.0f), 10.0f};
	mats.emplace_back(light);
	material_g metal = {METAL, Vec3(1.0f), Vec3(1.0f, 0.0f, 1.0f), 0.0f};
	mats.emplace_back(metal);
	material_g glass = {GLASS, Vec3(1.0f, 1.0f, 1.0f), Vec3(0.0f), 0.0f};
	mats.emplace_back(glass);
	material_g yellow = {LAMBERTIAN, Vec3(1.0f, 1.0f, 0.0f), Vec3(0.0f), 0.0f};
	mats.emplace_back(yellow);

	return mats;
}

std::vector<light_g> generate_lights() {
	std::vector<light_g> lights;
	light_g l1 = {3, 0.0f, 49.9f, 0.0f, 0.0f, 1.0f, 0.0f, 30.0f, 30.0f};
	lights.emplace_back(l1);

	return lights;
}

int main() {
	// GLFW initialization
	if (!glfwInit()) {
		std::cout << "GLFW failed to initialize" << std::endl;
		return -1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow *window = glfwCreateWindow(800, 600, "Path Tracer", nullptr, nullptr);
	if (!window) {
		std::cout << "Window creation failed" << std::endl;
		return -1;
	}
	glfwMakeContextCurrent(window);

	std::cout << glGetString(GL_VERSION) << " " << glGetString(GL_RENDERER) << std::endl;

	// GLEW initialization
	glewExperimental = GL_TRUE;
	glewInit();

	// imgui initialization
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 450");

	// scene building
	scene_params sp;

	auto materials = generate_materials();
	sp.materials = materials.data();
	sp.num_materials = materials.size();

	auto planes = generate_cornell_box();
	sp.planes = planes.data();
	sp.num_planes = planes.size();

	auto spheres = generate_balls();
	sp.spheres = spheres.data();
	sp.num_spheres = spheres.size();

	auto lights = generate_lights();
	sp.lights = lights.data();
	sp.num_lights = lights.size();

	float fov = 39.3f;
	float move_speed = 5.0f;

	Camera cam(Vec3(0.0f, 0.0f, -190.0), Vec3(0.0f, 0.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f), fov,
			   sp.width, sp.height);

	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, sp.width, sp.height, 0, GL_RGB, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	GLuint pbo;
	glGenBuffers(1, &pbo);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo);
	glBufferData(GL_PIXEL_UNPACK_BUFFER, sp.width * sp.height * 3 * sizeof(float), nullptr,
				 GL_DYNAMIC_DRAW);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

	cudaGraphicsResource *cuda_pbo;
	cudaGraphicsGLRegisterBuffer(&cuda_pbo, pbo, cudaGraphicsMapFlagsWriteDiscard);

	void *d_states = dstate_rng_init(16, sp.width, sp.height);

	int materials_size = sizeof(material_g) * sp.num_materials;
	int planes_size = sizeof(plane_g) * sp.num_planes;
	int sphere_size = sizeof(sphere_g) * sp.num_spheres;
	int lights_size = sizeof(light_g) * sp.num_lights;

	sp.materials = (material_g *)allocate_d_mem(sp.materials, materials_size);
	sp.planes = (plane_g *)allocate_d_mem(sp.planes, planes_size);
	sp.spheres = (sphere_g *)allocate_d_mem(sp.spheres, sphere_size);
	sp.lights = (light_g *)allocate_d_mem(sp.lights, lights_size);
	int num_frames = 1;

	int size = sp.width * sp.height * sizeof(float) * 3;
	float *accum_buffer_h = (float *)malloc(size);
	float *accum_buffer_d = (float *)allocate_d_mem(accum_buffer_h, size);

	// render loop
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("Controls");
		ImGui::SliderFloat("Camera FOV", &cam.fov, 10.0f, 180.0f);
		ImGui::SliderFloat("Move Speed", &move_speed, 0.5f, 50.0f);
		ImGui::SliderInt("Samples", &sp.samples, 1, 512);
		ImGui::Text("WASD to move, Q/E up/down");
		ImGui::End();

		if (!io.WantCaptureKeyboard) {
			if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
				cudaMemset(accum_buffer_d, 0, size);
				num_frames = 1;
				cam.position = cam.position + cam.forward * move_speed;
			}
			if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
				cudaMemset(accum_buffer_d, 0, size);
				num_frames = 1;
				cam.position = cam.position - cam.forward * move_speed;
			}
			if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
				cudaMemset(accum_buffer_d, 0, size);
				num_frames = 1;
				cam.position = cam.position - cam.right * move_speed;
			}
			if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
				cudaMemset(accum_buffer_d, 0, size);
				num_frames = 1;
				cam.position = cam.position + cam.right * move_speed;
			}
			if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
				cudaMemset(accum_buffer_d, 0, size);
				num_frames = 1;
				cam.position = cam.position + Vec3(0, 1, 0) * move_speed;
			}
			if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
				cudaMemset(accum_buffer_d, 0, size);
				num_frames = 1;
				cam.position = cam.position - Vec3(0, 1, 0) * move_speed;
			}
		}

		cam.updateBases();

		cudaGraphicsMapResources(1, &cuda_pbo, 0);
		size_t num_bytes;
		float *d_output;
		cudaGraphicsResourceGetMappedPointer((void **)&d_output, &num_bytes, cuda_pbo);

		render_gpu_interop(d_output, accum_buffer_d, num_frames, d_states, cam, sp);

		cudaGraphicsUnmapResources(1, &cuda_pbo, 0);

		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, sp.width, sp.height, GL_RGB, GL_FLOAT, nullptr);
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

		ImGui::Begin("Viewport");
		ImGui::Image((ImTextureID)(intptr_t)texture, ImVec2(sp.width, sp.height));
		ImGui::End();

		ImGui::Render();
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
		num_frames++;
	}

	free_d_mem(d_states);
	free_d_mem(accum_buffer_d);
	free_d_mem(sp.materials);
	free_d_mem(sp.planes);
	free_d_mem(sp.spheres);
	free_d_mem(sp.lights);

	cudaGraphicsUnregisterResource(cuda_pbo);
	glDeleteBuffers(1, &pbo);

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}
