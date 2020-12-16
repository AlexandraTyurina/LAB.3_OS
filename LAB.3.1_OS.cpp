#include <iostream>
#include <windows.h>
#include <iomanip>
#include <chrono>
#include <windows.h>

HANDLE* threads;
HANDLE* events;
const int numSteps = 100000000;
const int BLOCK_SIZE = 8307210;
const int maxBlocks = numSteps / BLOCK_SIZE + 1;

double Pi = 0;
long int blockNum;

DWORD WINAPI func_param(LPVOID lpParameter);

int main()
{
	int num_thread;
	int freeThread;
	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);
	std::cout << "\t Количество потоков: ";
	std::cin >> num_thread;
	while (num_thread!=0)
	{
		threads = new HANDLE[num_thread];
		events = new HANDLE[num_thread];
		auto start = std::chrono::system_clock::now();
		for (size_t i = 0; i < num_thread; i++)
		{
			threads[i] = CreateThread(NULL, 0, func_param, (LPVOID)i, CREATE_SUSPENDED, NULL);
			events[i] = CreateEventA(NULL, true, true, NULL);
		}
		blockNum = num_thread;
		while (blockNum < maxBlocks)
		{
			//получаем индекс в массиве handle
			freeThread = WaitForMultipleObjects(num_thread, events, false, INFINITE) - WAIT_OBJECT_0;
			ResumeThread(threads[freeThread]);
			ResetEvent(events[freeThread]);
		}
		for (size_t i = 0; i < num_thread; i++)
		{
			ResumeThread(threads[i]);
		}
		WaitForMultipleObjects(num_thread, threads, true, INFINITE);
		Pi = Pi / numSteps;
		auto end = std::chrono::system_clock::now();
		std::chrono::duration<double> elapsed = end - start;
		std::cout << " Значение числа pi: " << std::setprecision(13) << Pi << std::endl;
		std::cout << " Затраты по времени: " << elapsed.count() << "сек" << std::endl;
		for (size_t i = 0; i < num_thread; i++)
		{
			CloseHandle(threads[i]);
			CloseHandle(events[i]);
		}
		std::cout << "\t Количество потоков: ";
		std::cin >> num_thread;
	}
	return 0;
}
DWORD WINAPI func_param(LPVOID lpParameter)
{
	double sum = 0;
	double x = 0;
	int num = (int)lpParameter;
	while (true)
	{
		int start = num * BLOCK_SIZE;
		int end = (num + 1) * BLOCK_SIZE;
		if (end > numSteps)
			end = numSteps;
		for (size_t i = start; i < end; i++)
		{
			x = (i + 0.5) * (1.0 / numSteps);
			sum = sum + 4.0 / (1. + x * x);
		}
		SetEvent(events[(int)lpParameter]);

		num = InterlockedExchangeAdd(&blockNum, 1);
		if (blockNum > maxBlocks)
			break;
		SuspendThread(threads[(int)lpParameter]);
	}
	Pi += sum;
	return 0;
}