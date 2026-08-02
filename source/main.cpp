#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "graphics/mesh.h"
#include "graphics/shader.h"

#include <exception>
#include <iostream>
#include <vector>

namespace
{
	void framebufferSizeCallback(GLFWwindow*, int width, int height)
	{
		glViewport(0, 0, width, height);
	}
}

int main()
{
	if (!glfwInit())
	{
		std::cerr << "GLFW baslatilamadi.\n";
		return -1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(
		1280, 720, "Solar System Engine", nullptr, nullptr
	);

	if (window == nullptr)
	{
		std::cerr << "Pencere olusturulamadi.\n";
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	if (!gladLoadGLLoader(
		reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
	{
		std::cerr << "GLAD yuklenemedi.\n";
		glfwDestroyWindow(window);
		glfwTerminate();
		return -1;
	}

	const std::vector<float> triangleVertices = {
		 0.0f,  0.6f, 0.0f,
		-0.6f, -0.6f, 0.0f,
		 0.6f, -0.6f, 0.0f
	};

	int exitCode = 0;

	try
	{
		{
			shader shader("shaders/basic.vert", "shaders/basic.frag");
			mesh triangle(triangleVertices);

			while (!glfwWindowShouldClose(window))
			{
				if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
				{
					glfwSetWindowShouldClose(window, true);
				}

				glClearColor(0.01f, 0.01f, 0.04f, 1.0f);
				glClear(GL_COLOR_BUFFER_BIT);

				shader.use();
				triangle.draw();

				glfwSwapBuffers(window);
				glfwPollEvents();
			}
		}
	}
	catch (const std::exception& error)
	{
		std::cerr << error.what() << '\n';
		exitCode = -1;
	}

	glfwDestroyWindow(window);
	glfwTerminate();

	return exitCode;
}