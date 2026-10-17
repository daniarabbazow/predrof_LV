#include "Input.h"
#include "Part1.h"
#include "Part2.h"

#include <ctime>
#include <deque>
#include <list>
#include <vector>

// меню части 1 для одного типа контейнера (vector, deque или list).
// функция шаблонная, поэтому один и тот же код работает с любым из них
template <class Container>
void part1ContainerMenu(const std::string& p_containerName)
{
    Container data;

    while (true)
    {
        std::cout << std::endl
                  << "--- Part 1: " << p_containerName
                  << " (elements: " << data.size() << ") ---" << std::endl
                  << "1. Load data from " << INPUT_FILE << " (file stream)" << std::endl
                  << "2. Load data from " << INPUT_FILE << " (istream_iterator + copy)" << std::endl
                  << "3. Modify: template function with container" << std::endl
                  << "4. Modify: function with iterators" << std::endl
                  << "5. Modify: for_each + lambda" << std::endl
                  << "6. Show container" << std::endl
                  << "7. Save to " << OUTPUT_FILE << " (file stream)" << std::endl
                  << "8. Save to " << OUTPUT_FILE << " (ostream_iterator + copy)" << std::endl
                  << "0. Back" << std::endl;

        int choice = readInt("Your choice: ", 0, 8);

        // изменять можно только загруженные данные
        if (choice >= 3 && choice <= 5 && data.empty())
        {
            std::cout << "Container is empty, load data first" << std::endl;
            continue;
        }

        switch (choice)
        {
        case 0:
            return;

        case 1:
            data = loadDataStream<Container>(INPUT_FILE);
            std::cout << "Loaded " << data.size() << " numbers" << std::endl;
            break;

        case 2:
            data = loadDataCopy<Container>(INPUT_FILE);
            std::cout << "Loaded " << data.size() << " numbers" << std::endl;
            break;

        case 3:
            data = modify(data);
            std::cout << "Modified, sum and average added to the end" << std::endl;
            break;

        case 4:
            modify(data.begin(), data.end());
            addSumAndAverage(data);
            std::cout << "Modified, sum and average added to the end" << std::endl;
            break;

        case 5:
            modifyForEach(data);
            std::cout << "Modified, sum and average added to the end" << std::endl;
            break;

        case 6:
            printContainer(data);
            break;

        case 7:
            outputResultStream(data, OUTPUT_FILE);
            std::cout << "Saved to " << OUTPUT_FILE << std::endl;
            break;

        case 8:
            outputResultCopy(data, OUTPUT_FILE);
            std::cout << "Saved to " << OUTPUT_FILE << std::endl;
            break;
        }
    }
}

// главная функция: главное меню, из которого открываются части 1 и 2
int main()
{
    // чтобы случайные числа при каждом запуске были разными
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    while (true)
    {
        std::cout << std::endl
                  << "===== Homework 1: streams and STL (variant 1) =====" << std::endl
                  << "1. Part 1: create " << INPUT_FILE << " (file stream)" << std::endl
                  << "2. Part 1: create " << INPUT_FILE << " (generate + copy)" << std::endl
                  << "3. Part 1: work with std::vector" << std::endl
                  << "4. Part 1: work with std::deque" << std::endl
                  << "5. Part 1: work with std::list" << std::endl
                  << "6. Part 1: modify " << INPUT_FILE << " -> " << TRANSFORM_FILE
                  << " (transform)" << std::endl
                  << "7. Part 2: bank accounts" << std::endl
                  << "0. Exit" << std::endl;

        int choice = readInt("Your choice: ", 0, 7);

        switch (choice)
        {
        case 0:
            return 0;

        case 1:
            std::cout << "Created " << createDataStream(INPUT_FILE) << std::endl;
            break;

        case 2:
            std::cout << "Created " << createDataGenerate(INPUT_FILE) << std::endl;
            break;

        case 3:
            part1ContainerMenu<std::vector<double> >("std::vector");
            break;

        case 4:
            part1ContainerMenu<std::deque<double> >("std::deque");
            break;

        case 5:
            part1ContainerMenu<std::list<double> >("std::list");
            break;

        case 6:
            modifyTransform<double>(INPUT_FILE, TRANSFORM_FILE);
            std::cout << "Result saved to " << TRANSFORM_FILE << std::endl;
            break;

        case 7:
            part2Menu();
            break;
        }
    }
}
