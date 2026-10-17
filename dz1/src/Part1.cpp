#include "Part1.h"

#include <vector>

// сколько чисел записывается в файл
const int NUMBERS_COUNT = 100;

// случайное целое число от -50 до 50
int randomNumber()
{
    return std::rand() % 101 - 50;
}

// в цикле пишем 100 случайных чисел в файл, по одному на строке
std::string createDataStream(const std::string& p_fileName)
{
    std::ofstream file(p_fileName.c_str());
    if (!file)
    {
        std::cout << "Can not create file " << p_fileName << std::endl;
        return p_fileName;
    }

    for (int i = 0; i < NUMBERS_COUNT; ++i)
        file << randomNumber() << "\n";

    return p_fileName;
}

// generate заполняет вспомогательный вектор, copy выводит его в файл
std::string createDataGenerate(const std::string& p_fileName)
{
    std::ofstream file(p_fileName.c_str());
    if (!file)
    {
        std::cout << "Can not create file " << p_fileName << std::endl;
        return p_fileName;
    }

    std::vector<int> numbers(NUMBERS_COUNT);
    std::generate(numbers.begin(), numbers.end(), randomNumber);

    std::copy(numbers.begin(), numbers.end(), std::ostream_iterator<int>(file, "\n"));

    return p_fileName;
}
