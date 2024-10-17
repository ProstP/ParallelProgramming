﻿#include <Windows.h>
#include <iostream>
#include <string>
#include <fstream>

struct ThreadParams
{
	int num;
	clock_t start;
};

DWORD WINAPI ThreadProc(const LPVOID lpParams)
{
	const int operationNum = 10;
	ThreadParams params = *(ThreadParams*)lpParams;

	std::ofstream out("output.txt", std::ios::app);
	//std::ofstream out("output" + std::to_string(params.num) + ".txt");

	for (int i = 0; i < operationNum; i++)
	{
		const int maxValue = 500'000;
		for (int value = 3; value < maxValue; value++)
		{
			int multiply = 2;
			bool isPrime = true;
			while (maxValue / multiply > multiply && isPrime)
			{
				if (value % multiply == 0)
				{
					isPrime = false;
				}

				multiply++;
			}
		}

		//std::cout << std::to_string(num) + "|" + std::to_string((clock() - start) / (float)CLOCKS_PER_SEC) + "\n";
		out << std::to_string(params.num) + "|" + std::to_string((clock() -params.start) / (float)CLOCKS_PER_SEC) << std::endl;
		//out << std::to_string((clock() - params.start) / (float)CLOCKS_PER_SEC) << std::endl;
	}

	out.close();

	ExitThread(0);
}

void SetCoreCount(unsigned int num)
{
	SYSTEM_INFO info;
	GetSystemInfo(&info);
	int maxProccCount = info.dwNumberOfProcessors;

	if (num > maxProccCount)
	{
		num = maxProccCount;
	}

	HANDLE hProcess = GetCurrentProcess();
	DWORD_PTR mask = static_cast<DWORD_PTR>(pow(2, num) - 1);

	SetThreadAffinityMask(hProcess, mask);
}

int main()
{
	std::string c;
	std::cin >> c;

	auto start = clock();

	const int threadNum = 2;

	HANDLE* handles = new HANDLE[threadNum];
	ThreadParams* handleParams = new ThreadParams[threadNum];

	for (int i = 0; i < threadNum; i++)
	{
		handleParams[i].num = i + 1;
		handleParams[i].start = start;
		handles[i] = CreateThread(NULL, 0, &ThreadProc, &handleParams[i], CREATE_SUSPENDED, NULL);
		if (handles[i] == NULL)
		{
			std::cout << "Error in thread creating" << std::endl;
			return 1;
		}
	}

	for (int i = 0; i < threadNum; i++)
	{
		ResumeThread(handles[i]);
	}

	WaitForMultipleObjects(threadNum, handles, true, INFINITE);

	delete[] handles;
	delete[] handleParams;

	return 0;
}
