// enkiTS-Helloworld.cpp: 定义控制台应用程序的入口点。
//

#include <stdint.h>

#include <iostream>
#include <thread>
#include <stdlib.h>

#include "../enkiTS/TaskScheduler.h"

#include "Timer.h"

enki::TaskScheduler g_TS;

struct SerialTaskSet : enki::ITaskSet {
	long long int sum;

	SerialTaskSet(int size) 
	{ 
		m_SetSize = size; 
	}

	virtual void    ExecuteRange(enki::TaskSetPartition range, uint32_t threadnum) {
		sum = 0;
		for (int i = range.start; i < range.end; i++)
		{
			sum += i + 1;
		}
		//std::cout << "range: " << range.start << " " << range.end << std::endl;
		//std::cout << "m_SetSize: " << m_SetSize << " sum: " << sum << std::endl;
	}
};

struct ParallelTaskSet : enki::ITaskSet
{
	long long int*    sum;
	int partialNum;

	ParallelTaskSet(int size) 
	{
		m_SetSize = size;
	}

	// num of threads
	void Init(int _partialNum)
	{
		partialNum = _partialNum;
		sum = new long long int[partialNum];
		memset(sum, 0, sizeof(long long int)*partialNum);
	}

	virtual void    ExecuteRange(enki::TaskSetPartition range, uint32_t threadnum)
	{
		long long int tmpSum = sum[threadnum];
		for (int i = range.start; i < range.end; ++i)
		{
			tmpSum += i + 1;
		}
		sum[threadnum] = tmpSum;
	}

};

int main() {

	int size =  1024 * 1024 * 1024;
	Timer timer;

	// easy serial
	timer.Start();
	long long int sum = 0;
	for (int i = 0; i < size; i++)
	{
		sum += i + 1;
	}
	//std::cout << sizeof(int64_t) << std::endl;
	timer.Stop();
	std::cout << "easy Serial result:           " << sum <<" Time: "<< timer.getTime() << std::endl << std::endl;
	
	// enki serial explicit range
	timer.Reset();
	timer.Start();
	SerialTaskSet serialTask(1);
	enki::TaskSetPartition range = { 0, size };
	serialTask.ExecuteRange(range, 0);
	timer.Stop();
	std::cout << "enkiTS Serial explict result: " << serialTask.sum << " Time: " << timer.getTime() << std::endl << std::endl;

	// enki serial implict range
	timer.Reset();
	timer.Start();
	g_TS.Initialize(1);
	SerialTaskSet pserialTask(size);
	g_TS.AddTaskSetToPipe(&pserialTask);
	g_TS.WaitforTask(&pserialTask);
	timer.Stop();
	std::cout << "enkiTS Serial implict result: " << pserialTask.sum << " Time: " << timer.getTime() << std::endl << std::endl;

	// enki parallel
	timer.Reset();
	timer.Start();
	int maxThreads = std::thread::hardware_concurrency();
	g_TS.Initialize(maxThreads);
	long long int finalSum = 0;
	ParallelTaskSet parallelTaskSet(size);
	parallelTaskSet.Init(maxThreads);
	g_TS.AddTaskSetToPipe(&parallelTaskSet);
	g_TS.WaitforTask(&parallelTaskSet);
	for (int i = 0; i < 4; i++)
		finalSum += parallelTaskSet.sum[i];
	timer.Stop();
	std::cout << "enkiTS Parallel result:       " << finalSum << " Time: " << timer.getTime() << std::endl << std::endl;

	system("pause");

	return 0;
}

