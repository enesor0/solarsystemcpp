#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "core/simulation_clock.h"
#include "graphics/camera.h"
#include "graphics/orbit_path.h"
#include "graphics/ring.h"
#include "graphics/shader.h"
#include "graphics/sphere.h"
#include "graphics/starfield.h"
#include "graphics/texture.h"
#include "graphics/texture_library.h"
#include "world/planet.h"
#include "world/moon.h"
#include "world/solar_system.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <exception>
#include <array>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace
{
	const glm::vec3 initialCameraPosition(0.0f, 4.0f, 16.0f);
	const glm::vec3 initialCameraUp(0.0f, 1.0f, 0.0f);
	constexpr float initialCameraYaw = -90.0f;
	constexpr float initialCameraPitch = -14.0f;

	struct MouseInputState
	{
		Camera* camera = nullptr;
		bool firstPosition = true;
		float lastX = 0.0f;
		float lastY = 0.0f;
	};

	struct SimulationInputState
	{
		bool pauseKeyWasPressed = false;
		bool fasterKeyWasPressed = false;
		bool slowerKeyWasPressed = false;
	};

	struct FocusInputState
	{
		std::array<bool, 10> keyWasPressed{};
	};

	struct AtmosphereStyle
	{
		glm::vec3 color;
		float scale;
		float intensity;
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

	void updateWindowTitle(
		GLFWwindow* window,
		const SimulationClock& simulationClock)
	{
		std::ostringstream title;
		title << "Solar System Engine | "
			<< (simulationClock.isPaused() ? "DURAKLATILDI" : "CALISIYOR")
			<< " | Hiz x"
			<< std::fixed << std::setprecision(2)
			<< simulationClock.timeScale()
			<< " | Space: Duraklat | Yukari/Asagi: Hiz"
			<< " | 1-9: Odak | 0: Serbest Kamera";

		glfwSetWindowTitle(window, title.str().c_str());
	}

	void processInput(
		GLFWwindow* window,
		Camera& camera,
		SimulationClock& simulationClock,
		SimulationInputState& simulationInputState,
		FocusInputState& focusInputState,
		const SolarSystem& solarSystem,
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

		const bool pauseKeyIsPressed =
			glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;
		const bool fasterKeyIsPressed =
			glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS;
		const bool slowerKeyIsPressed =
			glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS;

		bool simulationChanged = false;

		if (pauseKeyIsPressed && !simulationInputState.pauseKeyWasPressed)
		{
			simulationClock.togglePause();
			simulationChanged = true;
		}

		if (fasterKeyIsPressed && !simulationInputState.fasterKeyWasPressed)
		{
			simulationClock.increaseSpeed();
			simulationChanged = true;
		}

		if (slowerKeyIsPressed && !simulationInputState.slowerKeyWasPressed)
		{
			simulationClock.decreaseSpeed();
			simulationChanged = true;
		}

		simulationInputState.pauseKeyWasPressed = pauseKeyIsPressed;
		simulationInputState.fasterKeyWasPressed = fasterKeyIsPressed;
		simulationInputState.slowerKeyWasPressed = slowerKeyIsPressed;

		if (simulationChanged)
		{
			updateWindowTitle(window, simulationClock);
		}

		constexpr std::array<int, 10> focusKeys{
			GLFW_KEY_0,
			GLFW_KEY_1,
			GLFW_KEY_2,
			GLFW_KEY_3,
			GLFW_KEY_4,
			GLFW_KEY_5,
			GLFW_KEY_6,
			GLFW_KEY_7,
			GLFW_KEY_8,
			GLFW_KEY_9
		};

		for (std::size_t keyIndex = 0; keyIndex < focusKeys.size(); ++keyIndex)
		{
			const bool keyIsPressed =
				glfwGetKey(window, focusKeys[keyIndex]) == GLFW_PRESS;

			if (keyIsPressed && !focusInputState.keyWasPressed[keyIndex])
			{
				if (keyIndex == 0)
				{
					camera.setPose(
						initialCameraPosition,
						initialCameraUp,
						initialCameraYaw,
						initialCameraPitch
					);
				}
				else
				{
					const std::size_t planetIndex = keyIndex - 1;

					if (planetIndex < solarSystem.planets().size())
					{
						const Planet& planet = solarSystem.planets().at(planetIndex);
						const float distance = 1.30f + planet.radius() * 6.0f;

						camera.focusOn(planet.position(), distance);
					}
				}
			}

			focusInputState.keyWasPressed[keyIndex] = keyIsPressed;
		}
	}

	void drawPlanet(
		Shader& shader,
		const Sphere& sphere,
		const Planet& planet,
		const Texture* texture = nullptr)
	{
		shader.use();
		shader.setMat4("model", planet.modelMatrix());
		shader.setVec3("objectColor", planet.baseColor());
		shader.setInt("useTexture", texture == nullptr ? 0 : 1);

		if (texture != nullptr)
		{
			texture->bind();
			shader.setInt("surfaceTexture", 0);
		}

		sphere.draw();
	}

	void drawSun(
		Shader& shader,
		const Sphere& sphere,
		const Planet& sun,
		const Texture& texture,
		float time)
	{
		shader.use();
		shader.setMat4("model", sun.modelMatrix());
		texture.bind();
		shader.setInt("sunTexture", 0);
		shader.setFloat("time", time);

		sphere.draw();
	}

	void drawSunHalo(
		Shader& shader,
		const Sphere& sphere,
		const Planet& sun,
		const Camera& camera,
		float time)
	{
		shader.use();

		const glm::mat4 haloModel = glm::scale(
			sun.modelMatrix(),
			glm::vec3(1.35f)
		);

		shader.setMat4("model", haloModel);
		shader.setVec3("viewPosition", camera.position());
		shader.setVec3("haloColor", glm::vec3(1.0f, 0.35f, 0.05f));
		shader.setFloat("time", time);

		sphere.draw();
	}

	const AtmosphereStyle* atmosphereForPlanet(const Planet& planet)
	{
		static const AtmosphereStyle earth{
			glm::vec3(0.16f, 0.48f, 1.00f), 1.10f, 0.34f
		};
		static const AtmosphereStyle venus{
			glm::vec3(1.00f, 0.70f, 0.25f), 1.06f, 0.14f
		};
		static const AtmosphereStyle uranus{
			glm::vec3(0.35f, 0.90f, 1.00f), 1.06f, 0.14f
		};
		static const AtmosphereStyle neptune{
			glm::vec3(0.15f, 0.45f, 1.00f), 1.07f, 0.18f
		};

		if (planet.name() == "Earth")
		{
			return &earth;
		}

		if (planet.name() == "Venus")
		{
			return &venus;
		}

		if (planet.name() == "Uranus")
		{
			return &uranus;
		}

		if (planet.name() == "Neptune")
		{
			return &neptune;
		}

		return nullptr;
	}

	void drawAtmosphere(
		Shader& shader,
		const Sphere& sphere,
		const Planet& planet,
		const Camera& camera,
		const AtmosphereStyle& style)
	{
		shader.use();

		const glm::mat4 atmosphereModel = glm::scale(
			planet.modelMatrix(),
			glm::vec3(style.scale)
		);

		shader.setMat4("model", atmosphereModel);
		shader.setVec3("viewPosition", camera.position());
		shader.setVec3("atmosphereColor", style.color);
		shader.setFloat("intensity", style.intensity);

		sphere.draw();
	}

	void drawOrbitPath(
		Shader& shader,
		const OrbitPath& orbitPath,
		const Planet& planet)
	{
		shader.use();
		shader.setMat4("model", planet.orbitPathModelMatrix());
		shader.setVec3("objectColor", glm::vec3(0.20f, 0.20f, 0.28f));

		orbitPath.draw();
	}

	void drawMoonOrbitPath(
		Shader& shader,
		const OrbitPath& orbitPath,
		const Moon& moon,
		const Planet& parentPlanet)
	{
		shader.use();
		shader.setMat4(
			"model",
			moon.orbitPathModelMatrix(parentPlanet.position())
		);
		shader.setVec3("objectColor", glm::vec3(0.34f, 0.34f, 0.40f));

		orbitPath.draw();
	}

	void drawRing(
		Shader& shader,
		const Ring& ring,
		const Planet& planet)
	{
		shader.use();
		shader.setMat4("model", planet.ringModelMatrix());
		shader.setVec3("ringColor", planet.ringColor());

		ring.draw();
	}

	void drawMoon(
		Shader& shader,
		const Sphere& sphere,
		const Moon& moon,
		const Planet& parentPlanet,
		const Texture* texture)
	{
		shader.use();
		shader.setMat4(
			"model",
			moon.modelMatrix(parentPlanet.position())
	);
		shader.setVec3("objectColor", moon.baseColor());
		shader.setInt("useTexture", texture == nullptr ? 0 : 1);

		if (texture != nullptr)
		{
			texture->bind();
			shader.setInt("surfaceTexture", 0);
		}

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
	glEnable(GL_PROGRAM_POINT_SIZE);

	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	Camera camera(
		initialCameraPosition,
		initialCameraUp,
		initialCameraYaw,
		initialCameraPitch
	);

	MouseInputState mouseState;
	mouseState.camera = &camera;

	glfwSetWindowUserPointer(window, &mouseState);
	glfwSetCursorPosCallback(window, mouseCallback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	float previousTime = static_cast<float>(glfwGetTime());
	SimulationClock simulationClock;
	SimulationInputState simulationInputState;
	FocusInputState focusInputState;

	updateWindowTitle(window, simulationClock);

	int exitCode = 0;

	try
	{
		Shader basicShader(
			"shaders/basic.vert",
			"shaders/basic.frag"
		);

		Shader planetShader(
			"shaders/planet.vert",
			"shaders/planet.frag"
		);

		Shader sunShader(
			"shaders/sun.vert",
			"shaders/sun.frag"
		);

		Shader sunHaloShader(
			"shaders/sun.vert",
			"shaders/sun_halo.frag"
		);

		Shader ringShader(
			"shaders/ring.vert",
			"shaders/ring.frag"
		);

		Shader atmosphereShader(
			"shaders/atmosphere.vert",
			"shaders/atmosphere.frag"
		);

		Shader starShader(
			"shaders/star.vert",
			"shaders/star.frag"
		);

		Sphere unitSphere(1.0f);
		OrbitPath unitOrbitPath;
		Ring saturnRing(0.62f, 1.00f);
		Starfield starfield;

		TextureLibrary surfaceTextures;
		surfaceTextures.load("Mercury", "assets/textures/mercury.png");
		surfaceTextures.load("Sun", "assets/textures/sun.png");
		surfaceTextures.load("Venus", "assets/textures/venus.png");
		surfaceTextures.load("Earth", "assets/textures/earth.png");
		surfaceTextures.load("Mars", "assets/textures/mars.png");
		surfaceTextures.load("Jupiter", "assets/textures/jupiter.png");
		surfaceTextures.load("Saturn", "assets/textures/saturn.png");
		surfaceTextures.load("Uranus", "assets/textures/uranus.png");
		surfaceTextures.load("Neptune", "assets/textures/neptune.png");
		surfaceTextures.load("Moon", "assets/textures/moon.png");

		SolarSystem solarSystem;

		while (!glfwWindowShouldClose(window))
		{
			const float currentTime = static_cast<float>(glfwGetTime());
			const float deltaTime = currentTime - previousTime;

			previousTime = currentTime;

			processInput(
				window,
				camera,
				simulationClock,
				simulationInputState,
				focusInputState,
				solarSystem,
				deltaTime
			);

			solarSystem.update(simulationClock.advance(deltaTime));

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

			glDepthMask(GL_FALSE);

			starShader.use();
			starShader.setMat4("view", view);
			starShader.setMat4("projection", projection);
			starfield.draw();

			glDepthMask(GL_TRUE);

			sunHaloShader.use();
			sunHaloShader.setMat4("view", view);
			sunHaloShader.setMat4("projection", projection);

			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);
			glDepthMask(GL_FALSE);
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);

			for (const Planet& planet : solarSystem.planets())
			{
				if (planet.emitsLight())
				{
					drawSunHalo(
						sunHaloShader,
						unitSphere,
						planet,
						camera,
						currentTime
					);
				}
			}

			glDisable(GL_CULL_FACE);
			glDepthMask(GL_TRUE);
			glDisable(GL_BLEND);

			basicShader.use();
			basicShader.setMat4("view", view);
			basicShader.setMat4("projection", projection);

			for (const Planet& planet : solarSystem.planets())
			{
				if (planet.orbitRadius() > 0.0f)
				{
					drawOrbitPath(
						basicShader,
						unitOrbitPath,
						planet
					);
				}
			}

			for (const Moon& moon : solarSystem.moons())
			{
				const Planet& parentPlanet = solarSystem.planets().at(
					moon.parentPlanetIndex()
				);

				drawMoonOrbitPath(
					basicShader,
					unitOrbitPath,
					moon,
					parentPlanet
				);
			}

			ringShader.use();
			ringShader.setMat4("view", view);
			ringShader.setMat4("projection", projection);

			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glDepthMask(GL_FALSE);

			for (const Planet& planet : solarSystem.planets())
			{
				if (planet.hasRing())
				{
					drawRing(ringShader, saturnRing, planet);
				}
			}

			glDepthMask(GL_TRUE);
			glDisable(GL_BLEND);

			atmosphereShader.use();
			atmosphereShader.setMat4("view", view);
			atmosphereShader.setMat4("projection", projection);

			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);
			glDepthMask(GL_FALSE);
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);

			for (const Planet& planet : solarSystem.planets())
			{
				const AtmosphereStyle* atmosphere = atmosphereForPlanet(planet);

				if (atmosphere != nullptr)
				{
					drawAtmosphere(
						atmosphereShader,
						unitSphere,
						planet,
						camera,
						*atmosphere
					);
				}
			}

			glDisable(GL_CULL_FACE);
			glDepthMask(GL_TRUE);
			glDisable(GL_BLEND);

			planetShader.use();
			planetShader.setMat4("view", view);
			planetShader.setMat4("projection", projection);
			planetShader.setVec3("lightPosition", glm::vec3(0.0f));
			planetShader.setVec3(
				"lightColor",
				glm::vec3(1.0f, 0.95f, 0.85f)
			);
			planetShader.setVec3("viewPosition", camera.position());

			for (const Planet& planet : solarSystem.planets())
			{
				if (!planet.emitsLight())
				{
					drawPlanet(
						planetShader,
						unitSphere,
						planet,
						surfaceTextures.find(planet.name())
					);
				}
			}

			for (const Moon& moon : solarSystem.moons())
			{
				const Planet& parentPlanet = solarSystem.planets().at(
					moon.parentPlanetIndex()
				);

				drawMoon(
					planetShader,
					unitSphere,
					moon,
					parentPlanet,
					surfaceTextures.find(moon.name())
				);
			}

			sunShader.use();
			sunShader.setMat4("view", view);
			sunShader.setMat4("projection", projection);

			for (const Planet& planet : solarSystem.planets())
			{
				if (planet.emitsLight())
				{
					const Texture* sunTexture = surfaceTextures.find("Sun");

					if (sunTexture != nullptr)
					{
						drawSun(
							sunShader,
							unitSphere,
							planet,
							*sunTexture,
							currentTime
						);
					}
				}
			}

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
