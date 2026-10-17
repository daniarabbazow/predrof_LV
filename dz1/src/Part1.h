#ifndef PART1_H
#define PART1_H

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>
#include <numeric>
#include <string>

// имена файлов части 1
const std::string INPUT_FILE = "input.txt";
const std::string OUTPUT_FILE = "output.txt";
const std::string TRANSFORM_FILE = "output_transform.txt";

// ---------------------------------------------------------- создание данных

// создаёт файл со 100 случайными числами от -50 до 50 через файловый поток,
// возвращает имя созданного файла
std::string createDataStream(const std::string& p_fileName);

// создаёт такой же файл, но через алгоритмы: generate заполняет вектор,
// copy выводит его в файл через потоковый итератор
std::string createDataGenerate(const std::string& p_fileName);

// --------------------------------------------------------- загрузка данных

// загружает числа из файла в контейнер через файловый поток (file >> value)
template <class Container>
Container loadDataStream(const std::string& p_fileName)
{
    typedef typename Container::value_type ValueType;

    Container data;
    std::ifstream file(p_fileName.c_str());
    if (!file)
    {
        std::cout << "Can not open file " << p_fileName << std::endl;
        return data;
    }

    ValueType value;
    while (file >> value)
        data.push_back(value);

    return data;
}

// загружает числа из файла в контейнер через потоковые итераторы и алгоритм copy
template <class Container>
Container loadDataCopy(const std::string& p_fileName)
{
    typedef typename Container::value_type ValueType;

    Container data;
    std::ifstream file(p_fileName.c_str());
    if (!file)
    {
        std::cout << "Can not open file " << p_fileName << std::endl;
        return data;
    }

    // istream_iterator читает числа из файла, back_inserter дописывает их в конец
    std::copy(std::istream_iterator<ValueType>(file),
              std::istream_iterator<ValueType>(),
              std::back_inserter(data));

    return data;
}

// ----------------------------------------------------- изменение данных

// считает число, которое по варианту 1 прибавляется к каждому элементу:
// корень из произведения максимума и последнего числа
// (берём модуль произведения, иначе корень был бы из отрицательного числа)
template <class Iter>
typename std::iterator_traits<Iter>::value_type getAddition(Iter p_first, Iter p_last)
{
    typedef typename std::iterator_traits<Iter>::value_type ValueType;

    if (p_first == p_last)
        return ValueType();

    ValueType maxValue = *std::max_element(p_first, p_last);
    ValueType lastValue = *std::prev(p_last);

    return std::sqrt(std::abs(maxValue * lastValue));
}

// добавляет в конец контейнера сумму и среднее арифметическое элементов по модулю
template <class Container>
void addSumAndAverage(Container& p_data)
{
    typedef typename Container::value_type ValueType;

    if (p_data.empty())
        return;

    ValueType sum = std::accumulate(p_data.begin(), p_data.end(), ValueType(),
        [](ValueType p_sum, ValueType p_value) { return p_sum + std::abs(p_value); });

    // среднее считаем до push_back, пока размер ещё не изменился
    ValueType average = sum / p_data.size();

    p_data.push_back(sum);
    p_data.push_back(average);
}

// версия 1: шаблонная функция, принимает контейнер и возвращает изменённую копию,
// в конец копии добавляются сумма и среднее по модулю
template <class Container>
Container modify(const Container& p_data)
{
    typedef typename Container::value_type ValueType;

    Container result(p_data);
    ValueType addition = getAddition(result.begin(), result.end());

    for (ValueType& value : result)
        value += addition;

    addSumAndAverage(result);
    return result;
}

// версия 2: принимает итераторы на начало и конец и меняет элементы на месте
template <class Iter>
void modify(Iter p_first, Iter p_last)
{
    typedef typename std::iterator_traits<Iter>::value_type ValueType;

    ValueType addition = getAddition(p_first, p_last);

    for (Iter it = p_first; it != p_last; ++it)
        *it += addition;
}

// версия 3: алгоритм for_each, операция над каждым элементом передаётся лямбдой,
// в конец добавляются сумма и среднее по модулю
template <class Container>
void modifyForEach(Container& p_data)
{
    typedef typename Container::value_type ValueType;

    ValueType addition = getAddition(p_data.begin(), p_data.end());

    std::for_each(p_data.begin(), p_data.end(),
        [addition](ValueType& p_value) { p_value += addition; });

    addSumAndAverage(p_data);
}

// версия 4: алгоритм transform и потоковые итераторы - числа читаются из файла,
// сразу меняются и пишутся в файл результата, в конце - сумма и среднее по модулю
template <class T>
void modifyTransform(const std::string& p_inputName, const std::string& p_outputName)
{
    std::ifstream input(p_inputName.c_str());
    if (!input)
    {
        std::cout << "Can not open file " << p_inputName << std::endl;
        return;
    }

    // первый проход по файлу: ищем максимум и последнее число
    T maxValue = T();
    T lastValue = T();
    bool isEmpty = true;

    std::for_each(std::istream_iterator<T>(input), std::istream_iterator<T>(),
        [&](const T& p_value)
        {
            if (isEmpty || p_value > maxValue)
                maxValue = p_value;
            lastValue = p_value;
            isEmpty = false;
        });

    if (isEmpty)
    {
        std::cout << "File " << p_inputName << " is empty" << std::endl;
        return;
    }

    T addition = std::sqrt(std::abs(maxValue * lastValue));

    // второй проход: возвращаемся в начало файла, читаем, меняем и сразу пишем
    input.clear();
    input.seekg(0);

    std::ofstream output(p_outputName.c_str());
    if (!output)
    {
        std::cout << "Can not create file " << p_outputName << std::endl;
        return;
    }

    T sum = T();
    size_t count = 0;

    std::transform(std::istream_iterator<T>(input), std::istream_iterator<T>(),
                   std::ostream_iterator<T>(output, "\n"),
        [&](const T& p_value) -> T
        {
            T result = p_value + addition;
            sum += std::abs(result);
            ++count;
            return result;
        });

    output << sum << "\n" << sum / count << "\n";
}

// ---------------------------------------------------------- вывод данных

// записывает контейнер в файл через файловый поток, по одному числу на строке
template <class Container>
void outputResultStream(const Container& p_data, const std::string& p_fileName)
{
    typedef typename Container::value_type ValueType;

    std::ofstream file(p_fileName.c_str());
    if (!file)
    {
        std::cout << "Can not create file " << p_fileName << std::endl;
        return;
    }

    for (const ValueType& value : p_data)
        file << value << "\n";
}

// записывает контейнер в файл алгоритмом copy через потоковый итератор
template <class Container>
void outputResultCopy(const Container& p_data, const std::string& p_fileName)
{
    typedef typename Container::value_type ValueType;

    std::ofstream file(p_fileName.c_str());
    if (!file)
    {
        std::cout << "Can not create file " << p_fileName << std::endl;
        return;
    }

    std::copy(p_data.begin(), p_data.end(), std::ostream_iterator<ValueType>(file, "\n"));
}

// печатает контейнер на экран в одну строку
template <class Container>
void printContainer(const Container& p_data)
{
    typedef typename Container::value_type ValueType;

    if (p_data.empty())
    {
        std::cout << "Container is empty" << std::endl;
        return;
    }

    std::copy(p_data.begin(), p_data.end(), std::ostream_iterator<ValueType>(std::cout, " "));
    std::cout << std::endl;
}

#endif // PART1_H
