#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "graphics/camera.h"
#include "graphics/shader.h"
#include "graphics/sphere.h"
#include "world/planet.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <exception>
#include <iostream>

namespace
{
	struct MouseInputState
	{
		Camera* camera = nullptr;
		bool firstPosition = true;
		float lastX = 0.0f;
		float lastY = 0.0f;
	};

	void framebufferSizeCallback(GLFWwindow*, int width, int height)
	{
		glViewport(0, 0, width, height);
	}

	void mouseCallback(GLFWwindow* window, double xPosition, double yPosition)
	{
		auto* mouseState = static_cast<MouseInputState*>(
			glfwGetWindowUserPointer(window)
			);

		if (mouseState == nullptr || mouseState->camera == nullptr)
		{
			return;
		}

		const float x = static_cast<float>(xPosition);
		const float y = static_cast<float>(yPosition);

		if (mouseState->firstPosition)
		{
			mouseState->lastX = x;
			mouseState->lastY = y;
			mouseState->firstPosition = false;
			return;
		}

		mouseState->camera->rotate(
			x - mouseState->lastX,
			mouseState->lastY - y
		);

		mouseState->lastX = x;
		mouseState->lastY = y;
	}

	void processInput(
		GLFWwindow* window,
		Camera& camera,
		float deltaTime)
	{
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, true);
		}

		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		{
			camera.move(CameraDirection::Forward, deltaTime);
		}

		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		{
			camera.move(CameraDirection::Backward, deltaTime);
		}

		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		{
			camera.move(CameraDirection::Left, deltaTime);
		}

		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		{
			camera.move(CameraDirection::Right, deltaTime);
		}
	}

	void drawPlanet(
		Shader& shader,
		const Sphere& sphere,
		const Planet& planet,
		const glm::vec3& color)
	{
		shader.setMat4("model", planet.modelMatrix());
		shader.setVec3("objectColor", color);

		sphere.draw();
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
		1280,
		720,
		"Solar System Engine",
		nullptr,
		nullptr
	);

	if (window == nullptr)
	{
		std::cerr << "Pencere olusturulamadi.\n";
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	if (!gladLoadGLLoader(
		reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
	{
		std::cerr << "GLAD yuklenemedi.\n";
		glfwDestroyWindow(window);
		glfwTerminate();
		return -1;
	}

	int framebufferWidth = 0;
	int framebufferHeight = 0;

	glfwGetFramebufferSize(
		window,
		&framebufferWidth,
		&framebufferHeight
	);

	glViewport(0, 0, framebufferWidth, framebufferHeight);
	glEnable(GL_DEPTH_TEST);

	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	Camera camera(
		glm::vec3(0.0f, 2.5f, 10.0f),
		glm::vec3(0.0f, 1.0f, 0.0f),
		-90.0f,
		-12.0f
	);

	MouseInputState mouseState;
	mouseState.camera = &camera;

	glfwSetWindowUserPointer(window, &mouseState);
	glfwSetCursorPosCallback(window, mouseCallback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	float previousTime = static_cast<float>(glfwGetTime());
	int exitCode = 0;

	try
	{
		Shader basicShader(
			"shaders/basic.vert",
			"shaders/basic.frag"
		);

		Sphere unitSphere(1.0f);

		Planet sun(
			PlanetDefinition{
				"Sun",
				1.0f,
				0.0f,
				10.0f,
				0.0f
			}
		);

		Planet earth(
			PlanetDefinition{
				"Earth",
				0.35f,
				3.5f,
				80.0f,
				20.0f
			}
		);

		while (!glfwWindowShouldClose(window))
		{
			const float currentTime = static_cast<float>(glfwGetTime());
			const float deltaTime = currentTime - previousTime;

			previousTime = currentTime;

			processInput(window, camera, deltaTime);

			sun.update(deltaTime);
			earth.update(deltaTime);

			glfwGetFramebufferSize(
				window,
				&framebufferWidth,
				&framebufferHeight
			);

			if (framebufferHeight == 0)
			{
				glfwPollEvents();
				continue;
			}

			const float aspectRatio =
				static_cast<float>(framebufferWidth)
				/ static_cast<float>(framebufferHeight);

			const glm::mat4 view = camera.viewMatrix();

			const glm::mat4 projection = glm::perspective(
				glm::radians(45.0f),
				aspectRatio,
				0.1f,
				100.0f
			);

			glClearColor(0.01f, 0.01f, 0.04f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			basicShader.use();
			basicShader.setMat4("view", view);
			basicShader.setMat4("projection", projection);

			drawPlanet(
				basicShader,
				unitSphere,
				sun,
				glm::vec3(1.0f, 0.55f, 0.08f)
			);

			drawPlanet(
				basicShader,
				unitSphere,
				earth,
				glm::vec3(0.10f, 0.35f, 1.0f)
			);

			glfwSwapBuffers(window);
			glfwPollEvents();
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