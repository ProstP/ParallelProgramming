#include <Windows.h>
#include <ctime>
#include <iostream>
#include <omp.h>

void SynchronousFor(int bound)
{
	double pi = 0;

	for (int i = 0; i < bound; i++)
	{
		double value = (i % 2 == 0 ? 1 : -1);
		value = value / (2 * i + 1);

		pi += value;
	}

	std::cout << "For(синхронно): " << pi << std::endl;
}

void ForInParallel(int bound)
{
	double pi = 0;

#pragma omp parallel
	for (int i = 0; i < bound; i++)
	{
		double value = (i % 2 == 0 ? 1 : -1);
		value = value / (2 * i + 1);

		pi += value;
	}

	std::cout << "For(в паралелльных потоках): " << pi << std::endl;
}

void ParallelFor(int bound)
{
	double pi = 0;

#pragma omp parallel for
	for (int i = 0; i < bound; i++)
	{
		double value = (i % 2 == 0 ? 1 : -1);
		value = value / (2 * i + 1);

#pragma omp atomic
		pi += value;
	}

	std::cout << "For(паралелльно с atomic): " << pi << std::endl;
}

void ParallelForWithReduction(int bound)
{
	double pi = 0;

#pragma omp parallel for reduction(+:pi)
	for (int i = 0; i < bound; i++)
	{
		double value = (i % 2 == 0 ? 1 : -1);
		value = value / (2 * i + 1);

		pi += value;
	}

	std::cout << "For(с reduction): " << pi << std::endl;
}

int main()
{
	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);

	auto start = clock();
	//SynchronousFor(1'000'000);
	//ForInParallel(1'000'000);
	//ParallelFor(1'000'000);
	//ParallelForWithReduction(1'000'000);
	std::cout << clock() - start / (float)CLOCKS_PER_SEC << std::endl;
}
