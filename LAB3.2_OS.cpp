#include <iostream>
#include <iomanip>
#include <chrono>
#include <cmath>
#include <omp.h>
#include <windows.h>

double funcOpenMP()
{
    
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251); 
    
    const long numSteps = 10000000;
    const long BLOCK_SIZE = 8307210;

    double pi = 0;
    double sum = 0;
    double x;
    int number_of_threads;

    std::cout << " Количество потоков: ";
    std::cin >> number_of_threads;
    
    auto start = std::chrono::system_clock::now();

#pragma omp parallel for private (x), reduction (+:sum), num_threads(number_of_threads), schedule (dynamic, BLOCK_SIZE)
    for (int i = 0; i < numSteps; i++)
    {
        x = (i + 0.5) * (1.0 / numSteps);
        sum = sum + 4.0 / (1. + x * x);
    }
    pi = sum * (1.0 / numSteps);
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    std::cout << " Значение числа pi: " << std::setprecision(13) << pi << std::endl;
    std::cout << " Затраты по времени (мс): " << elapsed.count() << "сек";
    
    return pi;
}

int main()
{
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251); 
    
    std::cout << "\t Вычисление числа pi с помощью технологии Open MP" << std::endl;
    funcOpenMP();
    return 0;
}