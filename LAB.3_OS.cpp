// 830721 - номер студ. билета

//https://xakep.ru/2011/01/10/55570/
//https://eax.me/winapi-threads/
//https://jakeroid.com/ru/blog/critical-section-cpp.html по крит. сек+обработка

#include <iostream>
#include <iomanip>
#include <windows.h>
#include <string>
#include <omp.h>

void funcWinAPI();
DWORD WINAPI lpStartAddress(LPVOID lpParameter);
double count(long Num);
double funcOpenMp();


const long N = 10000000;
const long BLOCK_SIZE = 8307210;

DWORD tlsIndex;
CRITICAL_SECTION CriticalSection;
double Pi = 0;
DWORD qThreads = 0;
HANDLE* threadArray;



int main()
{
	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);
	std::cout << "\tВыберите способ подсчета числа pi: " << std::endl;
	std::cout << "1) WinAPI" << std::endl;
	std::cout << "2) OpenMp" << std::endl;
	std::cout << "0) Выход" << std::endl;
	int choice;
	std::cin >> choice;
	while (choice != 0)
	{
		switch (choice)
		{
		case 1:
			funcWinAPI();
			break;
		case 2:
			funcOpenMp();
			break;
		case 0:
			break;
		}
		std::cout << " Выберите способ подсчета числа pi: " << std::endl;
		std::cin >> choice;
	}
	return 0;
}

void funcWinAPI()
{
	std::cout << " Количество потоков: ";
	std::cin >> qThreads;
	//Cоздание набора дескрипторов для каждого потока
	threadArray = new HANDLE[qThreads];
	//Указатель на переменную, которая принимает идентификатор потока
	DWORD* lpThreadId = new DWORD[qThreads];
	//Локальная память потока (TLS); безопасное хранение данных, динамическая
	tlsIndex = TlsAlloc();
	//Критическая секция — объект для синхронизации данных между потоками, т.е она не позволяет выполнять некие действия одновременно(инициализация,т.е создание объекта).
	InitializeCriticalSection(&CriticalSection);

	//Создание необходимого числа потоков.
	for (int i = 0; i < qThreads; ++i)
	{
		threadArray[i] = CreateThread(nullptr, 0, lpStartAddress, new long{ i * BLOCK_SIZE }, CREATE_SUSPENDED, lpThreadId + i);
	}

	unsigned int start = GetTickCount();

	// Возобновление выполнения потоков (из-за CREATE_SUSPENDED)
	for (int i = 0; i < qThreads; ++i)
	{
		ResumeThread(threadArray[i]);
	}
	//Функция ждет, когда объект перейдет в сигнальное состояние, при этом поток, вызвавший ее, не расходует процессорное время.
	//TRUE - функция ждет включения в сигнальное состояние всех объектов, INFINITE - максимальное время ожидания (ожидать изменения состояния объекта можно бесконечно)
	WaitForMultipleObjects(qThreads, threadArray, TRUE, INFINITE);
	unsigned int end = GetTickCount();

	std::cout << " Значение числа pi: " << std::setprecision(10) << Pi << std::endl;
	std::cout << " Затраты по времени (мс): " << end - start << std::endl;

	DeleteCriticalSection(&CriticalSection);
	//Закрытие всех дескрипторов
	for (int i = 0; i < qThreads; ++i)
		CloseHandle(threadArray[i]);

	delete[] threadArray;
	threadArray = nullptr;
	Pi = 0;
	//Освобождение памяти
	TlsFree(tlsIndex);
}
// Определяемая программой функция, которая служит как начальный адрес для потока.
DWORD WINAPI lpStartAddress(LPVOID lpParameter)
{

	long InitProgress = *(reinterpret_cast<long*>(lpParameter));
	long Progress = InitProgress;

	double tlPi = 0;
	//Функция TlsSetValue сохраняет значение установленного индекса TLS в слоте локальной памяти (TLS) вызывающего потока. 
	//Каждый поток процесса имеет свою собственную область памяти (слот) для каждого индекса TLS.
	TlsSetValue(tlsIndex, (LPVOID)&tlPi);

	bool end = false;

	for (int i = 1; !end; ++i)
	{
		tlPi = count(Progress);

		TlsSetValue(tlsIndex, (LPVOID)&tlPi);
		//После выполнения этой функции данный поток становится владельцем данной секции. Следующий поток, вызвав данную функцию, будет находиться в состоянии ожидания.
		EnterCriticalSection(&CriticalSection);
		{
			Pi += tlPi;
			Progress = qThreads * BLOCK_SIZE * i + InitProgress;
			if (Progress >= N)
				end = true;
		}
		//Поток, покидая участок кода, где он работал с защищенным ресурсом, должен вызвать функцию LeaveCriticalSection. Тем самым он уведомляет систему о том, что кабинка с данным ресурсом освободилась.
		LeaveCriticalSection(&CriticalSection);
	}
	return 0;
}

double count(long Num)
{
	double xi = 0;
	double pi = 0;
	for (long i = Num; i < Num + BLOCK_SIZE && i < N; ++i)
	{
		xi = (i + 0.5) * (1.0 / N);
		pi += ((4.0 / (1.0 + xi * xi)) * (1.0 / N));
	}
	return pi;
}

//Директива pragma omp parallel for указывает на то, что данный цикл следует разделить по итерациям между потоками.
//https://pawno-info.ru/threads/27-podvodnyx-kamnja-openmp-pri-programmirovanii-na-si.137400/

double funcOpenMp()
{
	std::cout << " Количество потоков: ";
	std::cin >> qThreads;
	double xi = 0;
	double pi = 0;
	unsigned int start = GetTickCount();
	//Директива pragma omp parallel for указывает на то, что данный цикл следует разделить по итерациям между потоками.
#pragma omp parallel num_threads(qThreads)
	{
#pragma omp for schedule(dynamic, BLOCK_SIZE) private (xi) reduction(+: pi)
		for (int i = 0; i < N; ++i)
		{
			xi = (i + 0.5) * (1.0 / N);
			pi += ((4.0 / (1.0 + xi * xi)) * (1.0 / N));
		}
	}
	unsigned int end = GetTickCount();

	std::cout << " Значение числа pi: " << std::setprecision(10) << pi << std::endl;
	std::cout << " Затраты по врмени (мс): " << end - start << std::endl;
	return pi;
}