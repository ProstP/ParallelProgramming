#include "tchar.h"
#include <fstream>
#include <iostream>
#include <string>
#include <windows.h>

HANDLE Mutex;

int ReadFromFile()
{
	WaitForSingleObject(Mutex, INFINITE);
	std::fstream myfile("balance.txt", std::ios_base::in);
	int result;
	myfile >> result;
	myfile.close();
	ReleaseMutex(Mutex);

	return result;
}

void WriteToFile(int data)
{
	WaitForSingleObject(Mutex, INFINITE);
	std::fstream myfile("balance.txt", std::ios_base::out);
	myfile << data << std::endl;
	myfile.close();
	ReleaseMutex(Mutex);
}

int GetBalance()
{
	return ReadFromFile();
}

void Deposit(int index)
{
	int money = 230;
	int balance = GetBalance();
	balance += money;

	WriteToFile(balance);
	printf("Balance after deposit: %d, index: %d\n", balance, index);
}

void Withdraw(int index)
{
	int money = 1000;
	Sleep(20);
	int balance = GetBalance();
	if (balance < money)
	{
		printf("Cannot withdraw money, balance lower than %d, index: %d\n", money, index);
		return;
	}

	balance -= money;
	WriteToFile(balance);
	printf("Balance after withdraw: %d, index: %d\n", balance, index);
}

DWORD WINAPI DoDeposit(CONST LPVOID lpParameter)
{
	Deposit((int)lpParameter);
	ExitThread(0);
}

DWORD WINAPI DoWithdraw(CONST LPVOID lpParameter)
{
	Withdraw((int)lpParameter);
	ExitThread(0);
}

int main()
{
	HANDLE* handles = new HANDLE[49];

	Mutex = CreateMutex(NULL, FALSE, (LPCWSTR)("FileMutex"));

	WriteToFile(0);

	SetProcessAffinityMask(GetCurrentProcess(), 1);
	for (int i = 0; i < 50; i++)
	{
		handles[i] = (i % 2 == 0)
			? CreateThread(NULL, 0, &DoDeposit, (LPVOID)i, CREATE_SUSPENDED, NULL)
			: CreateThread(NULL, 0, &DoWithdraw, (LPVOID)i, CREATE_SUSPENDED, NULL);
		ResumeThread(handles[i]);
	}

	WaitForMultipleObjects(50, handles, true, INFINITE);
	printf("Final Balance: %d\n", GetBalance());

	getchar();

	CloseHandle(Mutex);

	return 0;
}
