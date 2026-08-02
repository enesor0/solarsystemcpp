#include "core/simulation_clock.h"

#include <algorithm>

float SimulationClock::advance(float frameDeltaTime)
{
	if (paused_)
	{
		return 0.0f;
	}

	const float elapsedDays = frameDeltaTime
		* timeScale_
		* simulationDaysPerSecond_;
	elapsedDays_ += elapsedDays;
	return elapsedDays;
}

float SimulationClock::elapsedDays() const
{
	return elapsedDays_;
}

void SimulationClock::togglePause()
{
	paused_ = !paused_;
}

void SimulationClock::increaseSpeed()
{
	timeScale_ = std::min(timeScale_ * speedStep_, maxTimeScale_);
}

void SimulationClock::decreaseSpeed()
{
	timeScale_ = std::max(timeScale_ / speedStep_, minTimeScale_);
}

bool SimulationClock::isPaused() const
{
	return paused_;
}

float SimulationClock::timeScale() const
{
	return timeScale_;
}
