#ifndef INPUT_H
#define INPUT_H

#include <cstdlib>
#include <iostream>
#include <limits>
#include <string>

// если ввод закончился (например, закрыли консоль), завершаем программу,
// иначе меню крутилось бы бесконечно
inline void checkEndOfInput()
{
    if (std::cin.eof())
    {
        std::cout << std::endl << "End of input" << std::endl;
        std::exit(0);
    }
}

// читает целое число от p_min до p_max, пока не введут правильно
inline int readInt(const std::string& p_prompt, int p_min, int p_max)
{
    while (true)
    {
        std::cout << p_prompt;

        int value;
        if (std::cin >> value && value >= p_min && value <= p_max)
            return value;

        checkEndOfInput();
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Wrong input, enter a number from " << p_min
                  << " to " << p_max << std::endl;
    }
}

// читает дробное число от p_min до p_max, пока не введут правильно
inline double readDouble(const std::string& p_prompt, double p_min, double p_max)
{
    while (true)
    {
        std::cout << p_prompt;

        double value;
        if (std::cin >> value && value >= p_min && value <= p_max)
            return value;

        checkEndOfInput();
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        // границы печатаем как целые, иначе большое число выведется как 1e+12
        std::cout << "Wrong input, enter a number from " << static_cast<long long>(p_min)
                  << " to " << static_cast<long long>(p_max) << std::endl;
    }
}

// читает одно слово (без пробелов), например фамилию
inline std::string readWord(const std::string& p_prompt)
{
    std::cout << p_prompt;

    std::string word;
    if (!(std::cin >> word))
        checkEndOfInput();

    return word;
}

#endif // INPUT_H
