//sin(2(x-2))/x-0.365 at 6pi/7
#include <gl/glew.h>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <GLFW/glfw3.h>
#include "World.h"
#include <time.h>
#include <stdint.h>

void window_size_callback(GLFWwindow *window, int width, int height)
{
	World *w = reinterpret_cast<World *>(glfwGetWindowUserPointer(window));
	w->SetupCameraProjection(90.f, width, height);
}

int main(int argc, char **argv)
{
	World world;

	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, true);

	GLFWwindow *w = glfwCreateWindow(800, 600, "AtomSim", nullptr, nullptr);

	glfwMakeContextCurrent(w);
	glewExperimental = GL_TRUE;
	glewInit();
	glfwShowWindow(w);

	std::srand(time(nullptr));

	world.Initialize();
	
	world.SetConstants(100000, 100000000, 5.f);
	world.SetElectronVelocity(0, 0);
	world.SetNucleonVelocity(0, 0);
	world.SetGenerationDistance(20);
	world.SetMasses(1, 10000, 10000);
	world.SetRenderingParameters(glm::vec3(1, 0, 0), glm::vec3(0, 0, 1), glm::vec3(0, 1, 0), 10, 1, 10);

	world.SetupCameraLook(glm::vec3(1, 1, 1) * 500.f, glm::vec3(0, 0, 0));
	world.SetupCameraProjection(90.f, 800, 600);

	world.ClearAndGenerate(200, 1000, 200);

	glfwSetTime(0);
	float begin = glfwGetTime();

	glfwSetWindowUserPointer(w, &world);
	glfwSetWindowSizeCallback(w, window_size_callback);
	while (!glfwWindowShouldClose(w))
	{
		float end = glfwGetTime();

		glfwPollEvents();

		world.Simulate(end - begin);
		world.Render();

		glfwSwapBuffers(w);

		begin = end;
	}

	world.Dispose();

	glfwDestroyWindow(w);
	return 0;
}