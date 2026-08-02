#pragma once

class SimulationClock
{
public:
	float advance(float frameDeltaTime);
	float elapsedDays() const;

	void togglePause();
	void increaseSpeed();
	void decreaseSpeed();

	bool isPaused() const;
	float timeScale() const;

private:
	bool paused_ = false;
	float timeScale_ = 1.0f;
	float elapsedDays_ = 0.0f;

	static constexpr float simulationDaysPerSecond_ = 1.0f;
	static constexpr float minTimeScale_ = 0.25f;
	static constexpr float maxTimeScale_ = 16.0f;
	static constexpr float speedStep_ = 2.0f;
};
