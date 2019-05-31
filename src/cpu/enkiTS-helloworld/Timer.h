#pragma once

#include <windows.h>

class Timer
{
private:
	double clock;
	bool running;
	LARGE_INTEGER start;
public:
	Timer()
	{
		clock = 0.0;
		running = false;
	}

	void Start()
	{
		running = true;
		QueryPerformanceCounter(&start);
	}

	void Stop()
	{
		if (running)
		{
			clock += getTime();
			running = false;
		}
	}

	void Reset()
	{
		clock = 0.0;
		running = false;
	}

	double getTime()
	{
		if (!running)
		{
			return clock;
		}
 
		LARGE_INTEGER stop;
		QueryPerformanceCounter(&stop);
		LARGE_INTEGER clockdif;
		clockdif.QuadPart = stop.QuadPart - start.QuadPart;

		LARGE_INTEGER freq;
		QueryPerformanceFrequency(&freq);
		return (double)(1000 * clockdif.QuadPart) / (double)freq.QuadPart;
	}

	double getClock()
	{
		return clock + getTime();
	}
};


class DTimer
{
	double timeMS;
	bool   bRunning;
public:
	DTimer() : timeMS(0.0), bRunning(false) {}

	// start does not reset time to 0, use reset
	void Start()
	{
		bRunning = true;
		QueryPerformanceCounter(&start);
	}

	double GetTimeMS()
	{
		if (!bRunning)
		{
			return timeMS;
		}
		LARGE_INTEGER stop;
		QueryPerformanceCounter(&stop);
		LARGE_INTEGER elapsed;
		elapsed.QuadPart = stop.QuadPart - start.QuadPart;

		LARGE_INTEGER freq;
		QueryPerformanceFrequency(&freq);
		return (double)(1000 * elapsed.QuadPart) / (double)freq.QuadPart;
	};

	void Stop()
	{
		if (bRunning)
		{
			timeMS += GetTimeMS();
			bRunning = false;
		}
	}

	void Reset()
	{
		timeMS = 0.0;
		bRunning = false;
	}

private:
	LARGE_INTEGER start;
};
