#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "core/simulation_clock.h"
#include "graphics/asteroid_belt.h"
#include "graphics/camera.h"
#include "graphics/hud.h"
#include "graphics/orbit_path.h"
#include "graphics/orbit_trail.h"
#include "graphics/ring.h"
#include "graphics/shader.h"
#include "graphics/shadow_map.h"
#include "graphics/sphere.h"
#include "graphics/space_station_mesh.h"
#include "graphics/starfield.h"
#include "graphics/texture.h"
#include "graphics/texture_library.h"
#include "world/planet.h"
#include "world/moon.h"
#include "world/solar_system.h"
#include "world/planet_facts.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <exception>
#include <algorithm>
#include <array>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>

namespace
{
	const glm::vec3 initialCameraPosition(0.0f, 12.0f, 36.0f);
	const glm::vec3 initialCameraUp(0.0f, 1.0f, 0.0f);
	constexpr float initialCameraYaw = -90.0f;
	constexpr float initialCameraPitch = -18.0f;

	struct SimulationInputState
	{
		bool pauseKeyWasPressed = false;
		bool fasterKeyWasPressed = false;
		bool slowerKeyWasPressed = false;
	};

	struct FocusInputState
	{
		std::array<bool, 10> keyWasPressed{};
		std::optional<std::size_t> focusedPlanetIndex;
		std::string focusedPlanetName;
	float orbitAzimuth = 33.0f;
	float orbitElevation = 14.0f;
	bool isReturningToFreeCamera = false;
	bool titleNeedsUpdate = false;
	};

	struct OrbitInputState
	{
		bool keyWasPressed = false;
		bool showOrbitPaths = false;
	};

	struct HudInputState
	{
		bool keyWasPressed = false;
		bool isVisible = true;
	};

	struct PresentationInputState
	{
		bool keyWasPressed = false;
		bool isEnabled = false;
	};

	struct TrailInputState
	{
		bool keyWasPressed = false;
		bool isVisible = true;
		std::optional<std::size_t> trackedPlanetIndex;
	};

	struct DisplayModeState
	{
		bool escapeKeyWasPressed = false;
		bool fullscreenKeyWasPressed = false;
		bool isFullscreen = true;
		int windowedWidth = 1280;
		int windowedHeight = 720;
	};

	struct DisplayResolution
	{
		int width;
		int height;
	};

	constexpr std::array<DisplayResolution, 4> displayResolutions{
		DisplayResolution{ 1280, 720 },
		DisplayResolution{ 1600, 900 },
		DisplayResolution{ 1920, 1080 },
		DisplayResolution{ 2560, 1440 }
	};

	enum class GraphicsQuality
	{
		Performance,
		Balanced,
		Cinematic,
		Ultra
	};

	struct GraphicsProfile
	{
		const char* name;
		int shadowSampleCount;
		std::size_t asteroidCount;
		bool shadowsEnabled;
		bool atmospheresEnabled;
		bool cloudsEnabled;
		float haloIntensityMultiplier;
		float atmosphereIntensityMultiplier;
		float cloudOpacityMultiplier;
	};

	struct GraphicsSettingsState
	{
		GraphicsQuality quality = GraphicsQuality::Ultra;
		int resolutionIndex = 0;
		float fieldOfViewDegrees = 50.0f;
		float interfaceScale = 1.0f;
		bool vSyncEnabled = true;
		bool qualityKeyWasPressed = false;
		bool panelKeyWasPressed = false;
		bool resolutionKeyWasPressed = false;
		bool vSyncKeyWasPressed = false;
		bool fovDecreaseKeyWasPressed = false;
		bool fovIncreaseKeyWasPressed = false;
		bool interfaceDecreaseKeyWasPressed = false;
		bool interfaceIncreaseKeyWasPressed = false;
		bool isSettingsPanelVisible = false;
	};

	const GraphicsProfile& graphicsProfileFor(GraphicsQuality quality)
	{
		static const GraphicsProfile performance{
			"PERFORMANCE", 0, 140, false, false, false, 0.55f, 0.0f, 0.0f
		};
		static const GraphicsProfile balanced{
			"BALANCED", 6, 280, true, true, false, 0.78f, 0.72f, 0.0f
		};
		static const GraphicsProfile cinematic{
			"CINEMATIC", 10, 420, true, true, true, 1.0f, 0.92f, 0.82f
		};
		static const GraphicsProfile ultra{
			"ULTRA", 12, 560, true, true, true, 1.18f, 1.10f, 1.0f
		};

		switch (quality)
		{
		case GraphicsQuality::Performance: return performance;
		case GraphicsQuality::Balanced: return balanced;
		case GraphicsQuality::Cinematic: return cinematic;
		case GraphicsQuality::Ultra: return ultra;
		}

		return ultra;
	}

	GraphicsQuality nextGraphicsQuality(GraphicsQuality quality)
	{
		switch (quality)
		{
		case GraphicsQuality::Performance: return GraphicsQuality::Balanced;
		case GraphicsQuality::Balanced: return GraphicsQuality::Cinematic;
		case GraphicsQuality::Cinematic: return GraphicsQuality::Ultra;
		case GraphicsQuality::Ultra: return GraphicsQuality::Performance;
		}

		return GraphicsQuality::Ultra;
	}

	const DisplayResolution& selectedResolution(
		const GraphicsSettingsState& graphicsSettings)
	{
		return displayResolutions.at(
			static_cast<std::size_t>(graphicsSettings.resolutionIndex)
		);
	}

	HudGraphicsInfo makeHudGraphicsInfo(
		const GraphicsSettingsState& graphicsSettings)
	{
		const GraphicsProfile& profile = graphicsProfileFor(graphicsSettings.quality);
		return {
			profile.name,
			profile.shadowSampleCount,
			profile.asteroidCount,
			profile.shadowsEnabled,
			profile.atmospheresEnabled,
			profile.cloudsEnabled,
			selectedResolution(graphicsSettings).width,
			selectedResolution(graphicsSettings).height,
			graphicsSettings.vSyncEnabled,
			graphicsSettings.fieldOfViewDegrees,
			graphicsSettings.interfaceScale,
			graphicsSettings.isSettingsPanelVisible
		};
	}

	struct MouseInputState
	{
		Camera* camera = nullptr;
		FocusInputState* focusState = nullptr;
		bool firstPosition = true;
		float lastX = 0.0f;
		float lastY = 0.0f;
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

	void toggleFullscreen(GLFWwindow* window, DisplayModeState& displayState)
	{
		if (displayState.isFullscreen)
		{
			GLFWmonitor* monitor = glfwGetPrimaryMonitor();
			const GLFWvidmode* videoMode = monitor == nullptr
				? nullptr
				: glfwGetVideoMode(monitor);
			const int positionX = videoMode == nullptr
				? 100
				: (videoMode->width - displayState.windowedWidth) / 2;
			const int positionY = videoMode == nullptr
				? 100
				: (videoMode->height - displayState.windowedHeight) / 2;

			glfwSetWindowMonitor(
				window,
				nullptr,
				positionX,
				positionY,
				displayState.windowedWidth,
				displayState.windowedHeight,
				GLFW_DONT_CARE
			);
		}
		else
		{
			GLFWmonitor* monitor = glfwGetPrimaryMonitor();

			if (monitor == nullptr)
			{
				return;
			}

			const GLFWvidmode* videoMode = glfwGetVideoMode(monitor);

			if (videoMode == nullptr)
			{
				return;
			}

			glfwSetWindowMonitor(
				window,
				monitor,
				0,
				0,
				videoMode->width,
				videoMode->height,
				videoMode->refreshRate
			);
		}

		displayState.isFullscreen = !displayState.isFullscreen;
	}

	void applyDisplayResolution(
		GLFWwindow* window,
		DisplayModeState& displayState,
		const GraphicsSettingsState& graphicsSettings)
	{
		const DisplayResolution& resolution = selectedResolution(graphicsSettings);
		displayState.windowedWidth = resolution.width;
		displayState.windowedHeight = resolution.height;

		if (!displayState.isFullscreen)
		{
			glfwSetWindowSize(window, resolution.width, resolution.height);
			return;
		}

		GLFWmonitor* monitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* videoMode = monitor == nullptr
			? nullptr
			: glfwGetVideoMode(monitor);

		if (monitor != nullptr && videoMode != nullptr)
		{
			glfwSetWindowMonitor(
				window,
				monitor,
				0,
				0,
				resolution.width,
				resolution.height,
				videoMode->refreshRate
			);
		}
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

		const float xOffset = x - mouseState->lastX;
		const float yOffset = mouseState->lastY - y;

		if (mouseState->focusState != nullptr
			&& mouseState->focusState->focusedPlanetIndex.has_value())
		{
			mouseState->focusState->orbitAzimuth += xOffset * 0.25f;
			mouseState->focusState->orbitElevation = std::clamp(
				mouseState->focusState->orbitElevation + yOffset * 0.25f,
				-70.0f,
				70.0f
			);
		}
		else
		{
			mouseState->camera->rotate(xOffset, yOffset);
		}

		mouseState->lastX = x;
		mouseState->lastY = y;
	}

	void updateWindowTitle(
		GLFWwindow* window,
		const SimulationClock& simulationClock,
		const FocusInputState& focusInputState,
		const OrbitInputState& orbitInputState,
		const HudInputState& hudInputState,
		const PresentationInputState& presentationInputState,
		const TrailInputState& trailInputState,
		const DisplayModeState& displayState,
		const GraphicsSettingsState& graphicsSettings)
	{
		std::ostringstream title;
		title << "Solar System Engine | "
			<< (simulationClock.isPaused() ? "PAUSED" : "RUNNING")
			<< " | Speed x"
			<< std::fixed << std::setprecision(2)
			<< simulationClock.timeScale()
			<< " | Space: Pause | Up/Down: Speed"
			<< " | 1-9: Focus | 0: Free Camera"
			<< " | Focus: "
			<< (focusInputState.focusedPlanetIndex.has_value()
				? focusInputState.focusedPlanetName
				: "Free")
			<< " | O: Orbits "
			<< (orbitInputState.showOrbitPaths ? "On" : "Off")
			<< " | H: HUD "
			<< (hudInputState.isVisible ? "On" : "Off")
			<< " | M: Presentation "
			<< (presentationInputState.isEnabled ? "On" : "Off")
			<< " | T: Trail "
			<< (trailInputState.isVisible ? "On" : "Off")
			<< " | G: Graphics "
			<< graphicsProfileFor(graphicsSettings.quality).name
			<< " | FOV "
			<< static_cast<int>(graphicsSettings.fieldOfViewDegrees)
			<< " | V-Sync "
			<< (graphicsSettings.vSyncEnabled ? "On" : "Off")
			<< " | "
			<< (displayState.isFullscreen
				? "Esc: Windowed"
				: "F11: Fullscreen");

		glfwSetWindowTitle(window, title.str().c_str());
	}

	void processInput(
		GLFWwindow* window,
		Camera& camera,
		SimulationClock& simulationClock,
		SimulationInputState& simulationInputState,
		FocusInputState& focusInputState,
		OrbitInputState& orbitInputState,
		HudInputState& hudInputState,
		PresentationInputState& presentationInputState,
		TrailInputState& trailInputState,
		DisplayModeState& displayState,
		GraphicsSettingsState& graphicsSettings,
		const SolarSystem& solarSystem,
		float deltaTime)
	{
		const bool escapeKeyIsPressed =
			glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS;
		const bool fullscreenKeyIsPressed =
			glfwGetKey(window, GLFW_KEY_F11) == GLFW_PRESS;

		if (escapeKeyIsPressed && !displayState.escapeKeyWasPressed)
		{
			if (displayState.isFullscreen)
			{
				toggleFullscreen(window, displayState);
				focusInputState.titleNeedsUpdate = true;
			}
			else
			{
				glfwSetWindowShouldClose(window, true);
			}
		}

		if (fullscreenKeyIsPressed && !displayState.fullscreenKeyWasPressed)
		{
			toggleFullscreen(window, displayState);
			focusInputState.titleNeedsUpdate = true;
		}

		displayState.escapeKeyWasPressed = escapeKeyIsPressed;
		displayState.fullscreenKeyWasPressed = fullscreenKeyIsPressed;

		const bool movesForward = glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS;
		const bool movesBackward = glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS;
		const bool movesLeft = glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS;
		const bool movesRight = glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS;

		if ((movesForward || movesBackward || movesLeft || movesRight)
			&& focusInputState.focusedPlanetIndex.has_value())
		{
			focusInputState.focusedPlanetIndex.reset();
			focusInputState.focusedPlanetName.clear();
			focusInputState.isReturningToFreeCamera = false;
			focusInputState.titleNeedsUpdate = true;
		}

		if (movesForward)
		{
			camera.move(CameraDirection::Forward, deltaTime);
		}

		if (movesBackward)
		{
			camera.move(CameraDirection::Backward, deltaTime);
		}

		if (movesLeft)
		{
			camera.move(CameraDirection::Left, deltaTime);
		}

		if (movesRight)
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
					focusInputState.focusedPlanetIndex.reset();
					focusInputState.focusedPlanetName.clear();
					focusInputState.isReturningToFreeCamera = true;
				}
				else
				{
					const std::size_t planetIndex = keyIndex - 1;

					if (planetIndex < solarSystem.planets().size())
					{
						const Planet& planet = solarSystem.planets().at(planetIndex);

						focusInputState.focusedPlanetIndex = planetIndex;
						focusInputState.focusedPlanetName = planet.name();
						focusInputState.orbitAzimuth = 33.0f;
						focusInputState.orbitElevation = 14.0f;
						focusInputState.isReturningToFreeCamera = false;
					}
				}

				focusInputState.titleNeedsUpdate = true;
			}

			focusInputState.keyWasPressed[keyIndex] = keyIsPressed;
		}

		const bool orbitKeyIsPressed = glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS;

		if (orbitKeyIsPressed && !orbitInputState.keyWasPressed)
		{
			orbitInputState.showOrbitPaths = !orbitInputState.showOrbitPaths;
			focusInputState.titleNeedsUpdate = true;
		}

		orbitInputState.keyWasPressed = orbitKeyIsPressed;

		const bool hudKeyIsPressed = glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS;

		if (hudKeyIsPressed && !hudInputState.keyWasPressed)
		{
			hudInputState.isVisible = !hudInputState.isVisible;
			focusInputState.titleNeedsUpdate = true;
		}

		hudInputState.keyWasPressed = hudKeyIsPressed;

		const bool presentationKeyIsPressed =
			glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS;

		if (presentationKeyIsPressed && !presentationInputState.keyWasPressed)
		{
			presentationInputState.isEnabled =
				!presentationInputState.isEnabled;
			focusInputState.titleNeedsUpdate = true;
		}

		presentationInputState.keyWasPressed = presentationKeyIsPressed;

		const bool trailKeyIsPressed = glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS;

		if (trailKeyIsPressed && !trailInputState.keyWasPressed)
		{
			trailInputState.isVisible = !trailInputState.isVisible;
			focusInputState.titleNeedsUpdate = true;
		}

		trailInputState.keyWasPressed = trailKeyIsPressed;

		const bool graphicsKeyIsPressed = glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS;

		if (graphicsKeyIsPressed && !graphicsSettings.qualityKeyWasPressed)
		{
			graphicsSettings.quality = nextGraphicsQuality(graphicsSettings.quality);
			graphicsSettings.isSettingsPanelVisible = true;
			focusInputState.titleNeedsUpdate = true;
		}

		graphicsSettings.qualityKeyWasPressed = graphicsKeyIsPressed;

		const bool graphicsPanelKeyIsPressed =
			glfwGetKey(window, GLFW_KEY_F3) == GLFW_PRESS;

		if (graphicsPanelKeyIsPressed && !graphicsSettings.panelKeyWasPressed)
		{
			graphicsSettings.isSettingsPanelVisible =
				!graphicsSettings.isSettingsPanelVisible;
		}

		graphicsSettings.panelKeyWasPressed = graphicsPanelKeyIsPressed;

		const bool resolutionKeyIsPressed = glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS;

		if (resolutionKeyIsPressed && !graphicsSettings.resolutionKeyWasPressed)
		{
			graphicsSettings.resolutionIndex =
				(graphicsSettings.resolutionIndex + 1)
				% static_cast<int>(displayResolutions.size());
			applyDisplayResolution(window, displayState, graphicsSettings);
			graphicsSettings.isSettingsPanelVisible = true;
			focusInputState.titleNeedsUpdate = true;
		}

		graphicsSettings.resolutionKeyWasPressed = resolutionKeyIsPressed;

		const bool vSyncKeyIsPressed = glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS;

		if (vSyncKeyIsPressed && !graphicsSettings.vSyncKeyWasPressed)
		{
			graphicsSettings.vSyncEnabled = !graphicsSettings.vSyncEnabled;
			glfwSwapInterval(graphicsSettings.vSyncEnabled ? 1 : 0);
			graphicsSettings.isSettingsPanelVisible = true;
			focusInputState.titleNeedsUpdate = true;
		}

		graphicsSettings.vSyncKeyWasPressed = vSyncKeyIsPressed;

		const bool fovDecreaseKeyIsPressed =
			glfwGetKey(window, GLFW_KEY_LEFT_BRACKET) == GLFW_PRESS;
		const bool fovIncreaseKeyIsPressed =
			glfwGetKey(window, GLFW_KEY_RIGHT_BRACKET) == GLFW_PRESS;

		if (fovDecreaseKeyIsPressed && !graphicsSettings.fovDecreaseKeyWasPressed)
		{
			graphicsSettings.fieldOfViewDegrees = std::max(
				35.0f,
				graphicsSettings.fieldOfViewDegrees - 2.0f
			);
			graphicsSettings.isSettingsPanelVisible = true;
			focusInputState.titleNeedsUpdate = true;
		}

		if (fovIncreaseKeyIsPressed && !graphicsSettings.fovIncreaseKeyWasPressed)
		{
			graphicsSettings.fieldOfViewDegrees = std::min(
				90.0f,
				graphicsSettings.fieldOfViewDegrees + 2.0f
			);
			graphicsSettings.isSettingsPanelVisible = true;
			focusInputState.titleNeedsUpdate = true;
		}

		graphicsSettings.fovDecreaseKeyWasPressed = fovDecreaseKeyIsPressed;
		graphicsSettings.fovIncreaseKeyWasPressed = fovIncreaseKeyIsPressed;

		const bool interfaceDecreaseKeyIsPressed =
			glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS;
		const bool interfaceIncreaseKeyIsPressed =
			glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS;

		if (interfaceDecreaseKeyIsPressed
			&& !graphicsSettings.interfaceDecreaseKeyWasPressed)
		{
			graphicsSettings.interfaceScale = std::max(
				0.75f,
				graphicsSettings.interfaceScale - 0.05f
			);
			graphicsSettings.isSettingsPanelVisible = true;
		}

		if (interfaceIncreaseKeyIsPressed
			&& !graphicsSettings.interfaceIncreaseKeyWasPressed)
		{
			graphicsSettings.interfaceScale = std::min(
				1.35f,
				graphicsSettings.interfaceScale + 0.05f
			);
			graphicsSettings.isSettingsPanelVisible = true;
		}

		graphicsSettings.interfaceDecreaseKeyWasPressed = interfaceDecreaseKeyIsPressed;
		graphicsSettings.interfaceIncreaseKeyWasPressed = interfaceIncreaseKeyIsPressed;

		if (simulationChanged || focusInputState.titleNeedsUpdate)
		{
			updateWindowTitle(
				window,
				simulationClock,
				focusInputState,
				orbitInputState,
				hudInputState,
				presentationInputState,
				trailInputState,
				displayState,
				graphicsSettings
			);
			focusInputState.titleNeedsUpdate = false;
		}
	}

	void updateFocusedCamera(
		Camera& camera,
		FocusInputState& focusInputState,
		const SolarSystem& solarSystem,
		float deltaTime)
	{
		if (!focusInputState.focusedPlanetIndex.has_value())
		{
			if (focusInputState.isReturningToFreeCamera)
			{
				focusInputState.isReturningToFreeCamera = !camera.transitionToPose(
					initialCameraPosition,
					initialCameraUp,
					initialCameraYaw,
					initialCameraPitch,
					deltaTime
				);
			}
			return;
		}

		const std::size_t planetIndex = *focusInputState.focusedPlanetIndex;

		if (planetIndex >= solarSystem.planets().size())
		{
			focusInputState.focusedPlanetIndex.reset();
			focusInputState.focusedPlanetName.clear();
			focusInputState.titleNeedsUpdate = true;
			return;
		}

		const Planet& planet = solarSystem.planets().at(planetIndex);
		const float distance = 1.30f + planet.radius() * 6.0f;
		camera.followOrbitTarget(
			planet.position(),
			distance,
			focusInputState.orbitAzimuth,
			focusInputState.orbitElevation,
			deltaTime
		);
	}

	HudFocusInfo makeHudFocusInfo(
		const FocusInputState& focusInputState,
		const SolarSystem& solarSystem)
	{
		HudFocusInfo focusInfo;

		if (!focusInputState.focusedPlanetIndex.has_value())
		{
			return focusInfo;
		}

		const std::size_t planetIndex = *focusInputState.focusedPlanetIndex;

		if (planetIndex >= solarSystem.planets().size())
		{
			return focusInfo;
		}

		const Planet& planet = solarSystem.planets().at(planetIndex);
		focusInfo.name = planet.name();
		focusInfo.radius = planet.radius();
		focusInfo.orbitRadius = planet.orbitRadius();
		focusInfo.orbitSpeed = planet.orbitSpeed();

		const PlanetFacts& facts = planetFacts(planet.name());
		focusInfo.summary = facts.summary;
		focusInfo.diameterKilometers = facts.diameterKilometers;
		focusInfo.averageDistanceMillionKm = facts.averageDistanceMillionKm;

		for (const Moon& moon : solarSystem.moons())
		{
			if (moon.parentPlanetIndex() == planetIndex)
			{
				++focusInfo.moonCount;
			}
		}

		return focusInfo;
	}

	void updateFocusedTrail(
		OrbitTrail& trail,
		TrailInputState& trailInputState,
		const FocusInputState& focusInputState,
		const SolarSystem& solarSystem,
		float time)
	{
		if (!focusInputState.focusedPlanetIndex.has_value())
		{
			trail.reset();
			trailInputState.trackedPlanetIndex.reset();
			return;
		}

		const std::size_t planetIndex = *focusInputState.focusedPlanetIndex;

		if (planetIndex >= solarSystem.planets().size())
		{
			trail.reset();
			trailInputState.trackedPlanetIndex.reset();
			return;
		}

		if (trailInputState.trackedPlanetIndex != planetIndex)
		{
			trail.reset();
			trailInputState.trackedPlanetIndex = planetIndex;
		}

		trail.update(solarSystem.planets().at(planetIndex).position(), time);
	}

	struct SurfaceMaterial
	{
		float roughness;
		float metallic;
		float surfaceDetailStrength;
		float nightLightIntensity;
		float atmosphereFlowSpeed;
		float bandDetailStrength;
		float stormStrength;
	};

	SurfaceMaterial materialForPlanet(const Planet& planet)
	{
		if (planet.name() == "Earth") return { 0.34f, 0.0f, 4.0f, 0.34f, 0.0f, 0.0f, 0.0f };
		if (planet.name() == "Mercury") return { 0.82f, 0.0f, 5.0f, 0.0f, 0.0f, 0.0f, 0.0f };
		if (planet.name() == "Venus") return { 0.64f, 0.0f, 1.4f, 0.0f, 0.0f, 0.0f, 0.0f };
		if (planet.name() == "Mars") return { 0.78f, 0.0f, 5.4f, 0.0f, 0.0f, 0.0f, 0.0f };
		if (planet.name() == "Jupiter") return { 0.52f, 0.0f, 0.8f, 0.0f, 0.010f, 0.22f, 0.86f };
		if (planet.name() == "Saturn") return { 0.57f, 0.0f, 0.7f, 0.0f, 0.006f, 0.13f, 0.0f };
		if (planet.name() == "Uranus") return { 0.42f, 0.0f, 0.5f, 0.0f, 0.003f, 0.05f, 0.0f };
		if (planet.name() == "Neptune") return { 0.40f, 0.0f, 0.6f, 0.0f, 0.005f, 0.09f, 0.0f };

		return { 0.70f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f };
	}

	void drawPlanet(
		Shader& shader,
		const Sphere& sphere,
		const Planet& planet,
		const Texture* texture = nullptr,
		const Texture* normalTexture = nullptr,
		const Texture* roughnessTexture = nullptr,
		float time = 0.0f)
	{
		shader.use();
		shader.setMat4("model", planet.modelMatrix());
		shader.setVec3("objectColor", planet.baseColor());
		shader.setInt("useTexture", texture == nullptr ? 0 : 1);

		const SurfaceMaterial material = materialForPlanet(planet);
		shader.setFloat("roughness", material.roughness);
		shader.setFloat("metallic", material.metallic);
		shader.setFloat("surfaceDetailStrength", material.surfaceDetailStrength);
		shader.setFloat("nightLightIntensity", material.nightLightIntensity);
		shader.setFloat("time", time);
		shader.setFloat("atmosphereFlowSpeed", material.atmosphereFlowSpeed);
		shader.setFloat("bandDetailStrength", material.bandDetailStrength);
		shader.setFloat("stormStrength", material.stormStrength);
		shader.setInt("useNormalMap", normalTexture == nullptr ? 0 : 1);
		shader.setInt("useRoughnessMap", roughnessTexture == nullptr ? 0 : 1);

		if (texture != nullptr)
		{
			texture->bind();
			shader.setInt("surfaceTexture", 0);
		}

		if (normalTexture != nullptr)
		{
			normalTexture->bind(2);
			shader.setInt("normalMap", 2);
		}

		if (roughnessTexture != nullptr)
		{
			roughnessTexture->bind(3);
			shader.setInt("roughnessMap", 3);
		}

		sphere.draw();
	}

	void drawSun(
		Shader& shader,
		const Sphere& sphere,
		const Planet& sun,
		const Texture& texture,
		const Camera& camera,
		float time)
	{
		shader.use();
		shader.setMat4("model", sun.modelMatrix());
		texture.bind();
		shader.setInt("sunTexture", 0);
		shader.setFloat("time", time);
		shader.setVec3("viewPosition", camera.position());

		sphere.draw();
	}

	void drawSunHalo(
		Shader& shader,
		const Sphere& sphere,
		const Planet& sun,
		const Camera& camera,
		float time,
		float intensityMultiplier)
	{
		shader.use();

		shader.setVec3("viewPosition", camera.position());
		shader.setFloat("time", time);

		const auto drawLayer = [&shader, &sphere, &sun](
			float scale,
			const glm::vec3& color,
			float intensity)
		{
			shader.setMat4(
				"model",
				glm::scale(sun.modelMatrix(), glm::vec3(scale))
			);
			shader.setVec3("haloColor", color);
			shader.setFloat("haloIntensity", intensity);
			sphere.draw();
		};

		drawLayer(
			1.12f,
			glm::vec3(1.0f, 0.67f, 0.30f),
			0.072f * intensityMultiplier
		);
		drawLayer(
			1.34f,
			glm::vec3(1.0f, 0.31f, 0.065f),
			0.026f * intensityMultiplier
		);
		drawLayer(
			1.62f,
			glm::vec3(0.92f, 0.16f, 0.018f),
			0.008f * intensityMultiplier
		);
	}

	const AtmosphereStyle* atmosphereForPlanet(const Planet& planet)
	{
		static const AtmosphereStyle earth{
			glm::vec3(0.16f, 0.48f, 1.00f), 1.075f, 0.22f
		};
		static const AtmosphereStyle venus{
			glm::vec3(1.00f, 0.70f, 0.25f), 1.045f, 0.08f
		};
		static const AtmosphereStyle uranus{
			glm::vec3(0.35f, 0.90f, 1.00f), 1.045f, 0.08f
		};
		static const AtmosphereStyle neptune{
			glm::vec3(0.15f, 0.45f, 1.00f), 1.050f, 0.11f
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
		const AtmosphereStyle& style,
		float intensityMultiplier)
	{
		shader.use();

		const glm::mat4 atmosphereModel = glm::scale(
			planet.modelMatrix(),
			glm::vec3(style.scale)
		);

		shader.setMat4("model", atmosphereModel);
		shader.setVec3("viewPosition", camera.position());
		shader.setVec3("lightPosition", glm::vec3(0.0f));
		shader.setVec3("atmosphereColor", style.color);
		shader.setFloat("intensity", style.intensity * intensityMultiplier);

		sphere.draw();
	}

	void drawFocusOutline(
		Shader& shader,
		const Sphere& sphere,
		const Planet& planet,
		const Camera& camera,
		float time)
	{
		const float pulse = 1.11f + 0.015f * std::sin(time * 3.0f);
		const glm::mat4 outlineModel = glm::scale(
			planet.modelMatrix(),
			glm::vec3(pulse)
		);
		const glm::vec3 outlineColor = glm::mix(
			planet.baseColor(),
			glm::vec3(0.20f, 0.78f, 1.00f),
			0.68f
		);

		shader.use();
		shader.setMat4("model", outlineModel);
		shader.setVec3("viewPosition", camera.position());
		shader.setVec3("lightPosition", glm::vec3(0.0f));
		shader.setVec3("atmosphereColor", outlineColor);
		shader.setFloat("intensity", 0.32f);

		sphere.draw();
	}

	void drawEarthCloudLayer(
		Shader& shader,
		const Sphere& sphere,
		const Planet& earth,
		const Texture& earthTexture,
		const Camera& camera,
		float time,
		float opacityMultiplier)
	{
		shader.use();
		shader.setVec3("lightPosition", glm::vec3(0.0f));
		shader.setVec3("viewPosition", camera.position());
		shader.setFloat("time", time);
		shader.setFloat("cloudOpacity", opacityMultiplier);
		earthTexture.bind();
		shader.setInt("earthTexture", 0);

		const auto drawLayer = [&shader, &sphere, &earth](
			float scale,
			float offset,
			float coverage)
		{
			shader.setMat4(
				"model",
				glm::scale(earth.modelMatrix(), glm::vec3(scale))
			);
			shader.setFloat("cloudLayerOffset", offset);
			shader.setFloat("cloudCoverage", coverage);
			sphere.draw();
		};

		drawLayer(1.014f, 0.00f, 1.00f);
		drawLayer(1.027f, 0.19f, 0.46f);
	}

	void drawOrbitPath(
		Shader& shader,
		const OrbitPath& orbitPath,
		const Planet& planet)
	{
		shader.use();
		shader.setMat4("model", planet.orbitPathModelMatrix());
		shader.setVec3("objectColor", glm::vec3(0.15f, 0.18f, 0.28f));
		shader.setFloat("opacity", 0.38f);

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
		shader.setFloat("opacity", 0.28f);

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
		const Texture* texture,
		const Texture* normalTexture = nullptr,
		const Texture* roughnessTexture = nullptr)
	{
		shader.use();
		shader.setMat4(
			"model",
			moon.modelMatrix(parentPlanet.position())
	);
		shader.setVec3("objectColor", moon.baseColor());
		shader.setInt("useTexture", texture == nullptr ? 0 : 1);
		shader.setFloat(
			"roughness",
			moon.name() == "Moon" ? 0.94f : 0.84f
		);
		shader.setFloat("metallic", 0.0f);
		shader.setFloat("surfaceDetailStrength", 6.0f);
		shader.setFloat("nightLightIntensity", 0.0f);
		shader.setFloat("time", 0.0f);
		shader.setFloat("atmosphereFlowSpeed", 0.0f);
		shader.setFloat("bandDetailStrength", 0.0f);
		shader.setFloat("stormStrength", 0.0f);
		shader.setInt("useNormalMap", normalTexture == nullptr ? 0 : 1);
		shader.setInt("useRoughnessMap", roughnessTexture == nullptr ? 0 : 1);

		if (texture != nullptr)
		{
			texture->bind();
			shader.setInt("surfaceTexture", 0);
		}

		if (normalTexture != nullptr)
		{
			normalTexture->bind(2);
			shader.setInt("normalMap", 2);
		}

		if (roughnessTexture != nullptr)
		{
			roughnessTexture->bind(3);
			shader.setInt("roughnessMap", 3);
		}

		sphere.draw();
	}

	void drawSpaceStation(
		Shader& shader,
		const SpaceStationMesh& stationMesh,
		const SpaceStation& station,
		const Planet& parentPlanet)
	{
		shader.use();
		shader.setMat4("model", station.modelMatrix(parentPlanet.position()));
		shader.setInt("useTexture", 0);
		shader.setInt("useNormalMap", 0);
		shader.setInt("useRoughnessMap", 0);
		shader.setFloat("surfaceDetailStrength", 0.0f);
		shader.setFloat("nightLightIntensity", 0.0f);
		shader.setFloat("time", 0.0f);
		shader.setFloat("atmosphereFlowSpeed", 0.0f);
		shader.setFloat("bandDetailStrength", 0.0f);
		shader.setFloat("stormStrength", 0.0f);

		shader.setVec3("objectColor", glm::vec3(0.67f, 0.73f, 0.77f));
		shader.setFloat("roughness", 0.36f);
		shader.setFloat("metallic", 0.72f);
		stationMesh.drawModules();

		shader.setVec3("objectColor", glm::vec3(0.20f, 0.27f, 0.32f));
		shader.setFloat("roughness", 0.24f);
		shader.setFloat("metallic", 0.90f);
		stationMesh.drawTruss();

		shader.setVec3("objectColor", glm::vec3(0.015f, 0.085f, 0.40f));
		shader.setFloat("roughness", 0.28f);
		shader.setFloat("metallic", 0.24f);
		stationMesh.drawSolarPanels();

		shader.setVec3("objectColor", glm::vec3(0.72f, 0.76f, 0.78f));
		shader.setFloat("roughness", 0.42f);
		shader.setFloat("metallic", 0.66f);
		stationMesh.drawRadiators();

		shader.setVec3("objectColor", glm::vec3(0.025f, 0.045f, 0.070f));
		shader.setFloat("roughness", 0.25f);
		shader.setFloat("metallic", 0.80f);
		stationMesh.drawDetails();
	}

	void renderSunShadowMap(
		ShadowMap& shadowMap,
		Shader& depthShader,
		const Sphere& sphere,
		const SpaceStationMesh& stationMesh,
		const SolarSystem& solarSystem,
		int framebufferWidth,
		int framebufferHeight)
	{
		const glm::vec3 sunPosition(0.0f);
		const auto shadowMatrices = shadowMap.lightSpaceMatrices(sunPosition);

		depthShader.use();
		depthShader.setVec3("lightPosition", sunPosition);
		depthShader.setFloat("farPlane", shadowMap.farPlane());

		for (unsigned int face = 0; face < shadowMatrices.size(); ++face)
		{
			shadowMap.beginRender(face);
			depthShader.setMat4("shadowMatrix", shadowMatrices[face]);

			for (const Planet& planet : solarSystem.planets())
			{
				if (!planet.emitsLight())
				{
					depthShader.setMat4("model", planet.modelMatrix());
					sphere.draw();
				}
			}

			for (const Moon& moon : solarSystem.moons())
			{
				const Planet& parentPlanet = solarSystem.planets().at(
					moon.parentPlanetIndex()
				);

				depthShader.setMat4(
					"model",
					moon.modelMatrix(parentPlanet.position())
				);
				sphere.draw();
			}

			for (const SpaceStation& station : solarSystem.spaceStations())
			{
				if (station.parentPlanetIndex() < solarSystem.planets().size())
				{
					const Planet& parentPlanet = solarSystem.planets().at(
						station.parentPlanetIndex()
					);
					depthShader.setMat4(
						"model",
						station.modelMatrix(parentPlanet.position())
					);
					stationMesh.drawAll();
				}
			}
		}

		shadowMap.endRender(framebufferWidth, framebufferHeight);
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
	glfwWindowHint(GLFW_SAMPLES, 4);

	GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();

	if (primaryMonitor == nullptr)
	{
		std::cerr << "Birincil ekran bulunamadi.\n";
		glfwTerminate();
		return -1;
	}

	const GLFWvidmode* videoMode = glfwGetVideoMode(primaryMonitor);

	if (videoMode == nullptr)
	{
		std::cerr << "Ekran goruntu modu bulunamadi.\n";
		glfwTerminate();
		return -1;
	}

	glfwWindowHint(GLFW_REFRESH_RATE, videoMode->refreshRate);

	GLFWwindow* window = glfwCreateWindow(
		videoMode->width,
		videoMode->height,
		"Solar System Engine",
		primaryMonitor,
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
	glEnable(GL_MULTISAMPLE);

	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	Camera camera(
		initialCameraPosition,
		initialCameraUp,
		initialCameraYaw,
		initialCameraPitch
	);

	float previousTime = static_cast<float>(glfwGetTime());
	SimulationClock simulationClock;
	SimulationInputState simulationInputState;
	FocusInputState focusInputState;
	OrbitInputState orbitInputState;
	HudInputState hudInputState;
	PresentationInputState presentationInputState;
	TrailInputState trailInputState;
	DisplayModeState displayModeState;
	GraphicsSettingsState graphicsSettings;

	MouseInputState mouseState;
	mouseState.camera = &camera;
	mouseState.focusState = &focusInputState;

	glfwSetWindowUserPointer(window, &mouseState);
	glfwSetCursorPosCallback(window, mouseCallback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	updateWindowTitle(
		window,
		simulationClock,
		focusInputState,
		orbitInputState,
		hudInputState,
		presentationInputState,
		trailInputState,
		displayModeState,
		graphicsSettings
	);

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

		Shader shadowDepthShader(
			"shaders/shadow_depth.vert",
			"shaders/shadow_depth.frag"
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

		Shader cloudShader(
			"shaders/cloud.vert",
			"shaders/cloud.frag"
		);

		Shader starShader(
			"shaders/star.vert",
			"shaders/star.frag"
		);

		Shader asteroidShader(
			"shaders/asteroid.vert",
			"shaders/asteroid.frag"
		);

		Shader hudShader(
			"shaders/hud.vert",
			"shaders/hud.frag"
		);

		Sphere unitSphere(1.0f);
		OrbitPath unitOrbitPath;
		Ring saturnRing(0.62f, 1.00f);
		Starfield starfield(1800);
		AsteroidBelt asteroidBelt(560);
		SpaceStationMesh internationalSpaceStation;
		ShadowMap sunShadowMap;
		OrbitTrail focusedTrail;
		Hud hud;

		TextureLibrary surfaceTextures;
		TextureLibrary normalTextures;
		TextureLibrary roughnessTextures;
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

		normalTextures.load("Earth", "assets/textures/earth_normal.png");
		normalTextures.load("Mars", "assets/textures/mars_normal.png");
		normalTextures.load("Moon", "assets/textures/moon_normal.png");

		roughnessTextures.load("Earth", "assets/textures/earth_roughness.png");
		roughnessTextures.load("Mars", "assets/textures/mars_roughness.png");
		roughnessTextures.load("Moon", "assets/textures/moon_roughness.png");

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
				orbitInputState,
				hudInputState,
				presentationInputState,
				trailInputState,
				displayModeState,
				graphicsSettings,
				solarSystem,
				deltaTime
			);

			solarSystem.update(simulationClock.advance(deltaTime));
			updateFocusedCamera(
				camera,
				focusInputState,
				solarSystem,
				deltaTime
			);
			updateFocusedTrail(
				focusedTrail,
				trailInputState,
				focusInputState,
				solarSystem,
				currentTime
			);

			if (focusInputState.titleNeedsUpdate)
			{
				updateWindowTitle(
					window,
					simulationClock,
					focusInputState,
					orbitInputState,
					hudInputState,
					presentationInputState,
					trailInputState,
					displayModeState,
					graphicsSettings
				);
				focusInputState.titleNeedsUpdate = false;
			}

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

			const GraphicsProfile& graphicsProfile = graphicsProfileFor(
				graphicsSettings.quality
			);

			if (graphicsProfile.shadowsEnabled)
			{
				renderSunShadowMap(
					sunShadowMap,
					shadowDepthShader,
					unitSphere,
					internationalSpaceStation,
					solarSystem,
					framebufferWidth,
					framebufferHeight
				);
			}

			const glm::mat4 view = camera.viewMatrix();

			const glm::mat4 projection = glm::perspective(
				glm::radians(graphicsSettings.fieldOfViewDegrees),
				aspectRatio,
				0.1f,
				100.0f
			);

			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glDepthMask(GL_FALSE);

			starShader.use();
			starShader.setMat4("view", view);
			starShader.setMat4("projection", projection);
			starfield.draw();

			glDepthMask(GL_TRUE);

			if (graphicsProfile.asteroidCount > 0)
			{
				asteroidShader.use();
				asteroidShader.setMat4("model", glm::mat4(1.0f));
				asteroidShader.setMat4("view", view);
				asteroidShader.setMat4("projection", projection);
				asteroidShader.setFloat("time", simulationClock.elapsedDays());
				asteroidShader.setVec3("lightPosition", glm::vec3(0.0f));
				asteroidShader.setVec3("viewPosition", camera.position());
				constexpr std::size_t maxAsteroidExclusionBodies = 9;
				const std::size_t asteroidExclusionCount = std::min(
					solarSystem.planets().size(),
					maxAsteroidExclusionBodies
				);
				asteroidShader.setInt(
					"exclusionCount",
					static_cast<int>(asteroidExclusionCount)
				);

				for (std::size_t exclusionIndex = 0;
					exclusionIndex < asteroidExclusionCount;
					++exclusionIndex)
				{
					const Planet& planet = solarSystem.planets().at(
						exclusionIndex
					);
					const std::string uniformIndex = "["
						+ std::to_string(exclusionIndex) + "]";
					asteroidShader.setVec3(
						"exclusionCenters" + uniformIndex,
						planet.position()
					);
					asteroidShader.setFloat(
						"exclusionRadii" + uniformIndex,
						planet.radius() + 0.10f
					);
				}
				asteroidBelt.draw(graphicsProfile.asteroidCount);
			}

			if (trailInputState.isVisible
				&& focusInputState.focusedPlanetIndex.has_value()
				&& !presentationInputState.isEnabled)
			{
				basicShader.use();
				basicShader.setMat4("model", glm::mat4(1.0f));
				basicShader.setMat4("view", view);
				basicShader.setMat4("projection", projection);
				basicShader.setVec3("objectColor", glm::vec3(0.18f, 0.72f, 1.00f));
				basicShader.setFloat("opacity", 0.52f);

				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				glDepthMask(GL_FALSE);
				focusedTrail.draw();
				glDepthMask(GL_TRUE);
				glDisable(GL_BLEND);
			}

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
						currentTime,
						graphicsProfile.haloIntensityMultiplier
					);
				}
			}

			glDisable(GL_CULL_FACE);
			glDepthMask(GL_TRUE);
			glDisable(GL_BLEND);

			if (orbitInputState.showOrbitPaths
				&& !presentationInputState.isEnabled)
			{
				basicShader.use();
				basicShader.setMat4("view", view);
				basicShader.setMat4("projection", projection);

				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				glDepthMask(GL_FALSE);

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

				glDepthMask(GL_TRUE);
				glDisable(GL_BLEND);
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

			if (graphicsProfile.atmospheresEnabled)
			{
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
							*atmosphere,
							graphicsProfile.atmosphereIntensityMultiplier
						);
					}
				}
			}

			if (focusInputState.focusedPlanetIndex.has_value()
				&& !presentationInputState.isEnabled)
			{
				const std::size_t planetIndex = *focusInputState.focusedPlanetIndex;

				if (planetIndex < solarSystem.planets().size())
				{
					drawFocusOutline(
						atmosphereShader,
						unitSphere,
						solarSystem.planets().at(planetIndex),
						camera,
						currentTime
					);
				}
			}

			glDisable(GL_CULL_FACE);
			glDepthMask(GL_TRUE);
			glDisable(GL_BLEND);

			planetShader.use();
			sunShadowMap.bind(1);
			planetShader.setMat4("view", view);
			planetShader.setMat4("projection", projection);
			planetShader.setVec3("lightPosition", glm::vec3(0.0f));
			planetShader.setVec3(
				"lightColor",
				glm::vec3(9.0f, 8.1f, 7.4f)
			);
			planetShader.setVec3("viewPosition", camera.position());
			planetShader.setInt("shadowMap", 1);
			planetShader.setFloat("shadowFarPlane", sunShadowMap.farPlane());
			planetShader.setInt(
				"shadowsEnabled",
				graphicsProfile.shadowsEnabled ? 1 : 0
			);
			planetShader.setInt(
				"shadowSampleCount",
				graphicsProfile.shadowSampleCount
			);

			for (const Planet& planet : solarSystem.planets())
			{
				if (!planet.emitsLight())
				{
					drawPlanet(
						planetShader,
						unitSphere,
						planet,
						surfaceTextures.find(planet.name()),
						normalTextures.find(planet.name()),
						roughnessTextures.find(planet.name()),
						currentTime
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
					surfaceTextures.find(moon.name()),
					normalTextures.find(moon.name()),
					roughnessTextures.find(moon.name())
				);
			}

			for (const SpaceStation& station : solarSystem.spaceStations())
			{
				if (station.parentPlanetIndex() < solarSystem.planets().size())
				{
					drawSpaceStation(
						planetShader,
						internationalSpaceStation,
						station,
						solarSystem.planets().at(station.parentPlanetIndex())
					);
				}
			}

			cloudShader.use();
			cloudShader.setMat4("view", view);
			cloudShader.setMat4("projection", projection);

			const Texture* earthTexture = surfaceTextures.find("Earth");

			if (graphicsProfile.cloudsEnabled && earthTexture != nullptr)
			{
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				glDepthMask(GL_FALSE);

				for (const Planet& planet : solarSystem.planets())
				{
					if (planet.name() == "Earth")
					{
						drawEarthCloudLayer(
							cloudShader,
							unitSphere,
							planet,
							*earthTexture,
							camera,
							currentTime,
						graphicsProfile.cloudOpacityMultiplier
						);
					}
				}

				glDepthMask(GL_TRUE);
				glDisable(GL_BLEND);
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
							camera,
							currentTime
						);
					}
				}
			}

			if (hudInputState.isVisible && !presentationInputState.isEnabled)
			{
				const HudFocusInfo focusInfo = makeHudFocusInfo(
					focusInputState,
					solarSystem
				);

				hud.render(
					hudShader,
					framebufferWidth,
					framebufferHeight,
					focusInfo,
					simulationClock.isPaused(),
					simulationClock.timeScale(),
					orbitInputState.showOrbitPaths,
					makeHudGraphicsInfo(graphicsSettings),
					graphicsSettings.interfaceScale
				);
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
