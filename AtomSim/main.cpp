//sin(2(x-2))/x-0.365 at 6pi/7
#include <gl/glew.h>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <GLFW/glfw3.h>
#include "World.h"
#include <time.h>
#include <stdint.h>

float oldX, oldY;
bool mouseDown = false;
bool mouseAlternateDown = false;
bool mouseMiddleDown = false;

void window_size_callback(GLFWwindow *window, int width, int height)
{
	World *w = reinterpret_cast<World *>(glfwGetWindowUserPointer(window));
	w->SetupCameraProjection(90.f, width, height);
}

void cursor_position_callback(GLFWwindow *window, double xpos, double ypos)
{
	World *w = reinterpret_cast<World*>(glfwGetWindowUserPointer(window));
	float dX = xpos - oldX;
	float dY = ypos - oldY;
	if (mouseDown)
		w->RotateAround(dY * 0.005f, -dX * 0.005f);
	else if (mouseAlternateDown)
		w->ScaleDown(dY * 2.f);
	else if (mouseMiddleDown)
		w->Translate(dX * 1.f, dY * 1.f);
	oldX = xpos;
	oldY = ypos;
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
	if (action == GLFW_PRESS)
	{
		switch (button)
		{
		case GLFW_MOUSE_BUTTON_LEFT:
			mouseDown = true;
			break;
		case GLFW_MOUSE_BUTTON_MIDDLE:
			mouseMiddleDown = true;
			break;
		case GLFW_MOUSE_BUTTON_RIGHT:
			mouseAlternateDown = true;
			break;
		}
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}
	else if (action == GLFW_RELEASE)
	{
		switch (button)
		{
		case GLFW_MOUSE_BUTTON_LEFT:
			mouseDown = false;
			break;
		case GLFW_MOUSE_BUTTON_MIDDLE:
			mouseMiddleDown = false;
			break;
		case GLFW_MOUSE_BUTTON_RIGHT:
			mouseAlternateDown = false;
			break;
		}
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
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
	
	world.SetConstants(0.00005f, 300.f, 900.f);
	world.SetElectronVelocity(0, 10);
	world.SetNucleonVelocity(0, 10);
	world.SetGenerationDistance(300.f);
	world.SetMasses(0.005f, 5.f, 5.5f);
	world.SetRenderingParameters(glm::vec3(1, 0, 0), glm::vec3(0, 0, 1), glm::vec3(0, 1, 0), 10, 1, 10);

	world.SetupCameraLook(glm::vec3(1, 1, 1) * 500.f, glm::vec3(0, 0, 0));
	world.SetupCameraProjection(90.f, 800, 600);

	world.ClearAndGenerate(400, 200, 200);

	glfwSetTime(0);
	float begin = glfwGetTime();

	glfwSetWindowUserPointer(w, &world);
	glfwSetWindowSizeCallback(w, window_size_callback);
	glfwSetCursorPosCallback(w, cursor_position_callback);
	glfwSetMouseButtonCallback(w, mouse_button_callback);
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