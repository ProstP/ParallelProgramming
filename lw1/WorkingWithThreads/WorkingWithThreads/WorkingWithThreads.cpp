#include <Windows.h>
#include <iostream>
#include <string>

DWORD WINAPI ThreadProc(const LPVOID lpParams)
{
	int num = *(int*)lpParams;

	std::cout << "Поток #" << num << " выполняет своою работу" << std::endl;

	ExitThread(0);
}

int main(int argc, char* argv[])
{
	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);

	int threadCount;
	if (argc < 2)
	{
		std::cout << "Введите количество потоков, которые вы хотите создать: ";
		std::cin >> threadCount;
		std::cout << std::endl;
	}
	else
	{
		threadCount = std::stoi(argv[1]);
	}

	HANDLE* handles = new HANDLE[threadCount];
	int* handleNums = new int[threadCount];

	for (int i = 0; i < threadCount; i++)
	{
		handleNums[i] = i;
		handles[i] = CreateThread(NULL, 0, &ThreadProc, &handleNums[i], CREATE_SUSPENDED, NULL);
		if (handles[i] == NULL)
		{
			std::cout << "Ошибка при создании потока" << std::endl;
			return 1;
		}
	}

	for (int i = 0; i < threadCount; i++)
	{
		ResumeThread(handles[i]);
	}

	WaitForMultipleObjects(threadCount, handles, true, INFINITE);

	delete[] handles;
	delete[] handleNums;
	return 0;
}
