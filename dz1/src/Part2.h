#ifndef PART2_H
#define PART2_H

#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <utility>

// имена файлов части 2
const std::string ACCOUNTS_FILE = "accounts.bin";
const std::string SEARCH_FILE = "search_result.bin";
const std::string REPORT_FILE = "report.txt";
const std::string TEMP_FILE = "accounts.tmp";

// длина фамилии вместе с завершающим нулём
const int OWNER_SIZE = 32;

// дата
struct Date
{
    int day;
    int month;
    int year;
};

// счёт в банке (задание 21).
// фамилия хранится в массиве char фиксированной длины, а не в std::string,
// чтобы структуру можно было записать в бинарный файл целиком, одним write
struct Account
{
    int number;              // номер счёта
    int code;                // код счёта
    char owner[OWNER_SIZE];  // фамилия владельца
    double amount;           // сумма на счёте
    Date openDate;           // дата открытия счёта
    double percent;          // годовой процент начисления
};

// читает из бинарного файла одну запись целиком, false - если записей больше нет
bool readAccount(std::istream& p_file, Account& o_account);

// записывает в бинарный файл одну запись целиком
void writeAccount(std::ostream& p_file, const Account& p_account);

// вводит один счёт с клавиатуры
Account inputAccount();

// вводит счета с клавиатуры и дописывает их в конец файла
void addAccounts(const std::string& p_fileName);

// строка-заголовок таблицы счетов
std::string tableHeader();

// один счёт в виде строки таблицы
std::string accountToString(const Account& p_account);

// печатает все счета из файла на экран, возвращает их количество
int showAccounts(const std::string& p_fileName);

// удаляет счёт с заданным номером: файл переписывается в новый без этой записи
bool deleteAccount(const std::string& p_fileName, int p_number);

// заменяет счёт с заданным номером: файл переписывается в новый с новой записью
bool replaceAccount(const std::string& p_fileName, int p_number, const Account& p_newAccount);

// делает из бинарного файла текстовую ведомость с итоговой строкой
void makeReport(const std::string& p_binaryName, const std::string& p_textName);

// индекс для поиска по номеру счёта: номер -> номер записи в файле
std::multimap<int, long> buildNumberIndex(const std::string& p_fileName);

// индекс для поиска по коду счёта: код -> номер записи в файле
std::multimap<int, long> buildCodeIndex(const std::string& p_fileName);

// индекс для поиска по владельцу: фамилия -> номер записи в файле
std::multimap<std::string, long> buildOwnerIndex(const std::string& p_fileName);

// меню части 2
void part2Menu();

// строит индекс: читает все записи и запоминает, в какой по счёту записи
// лежит каждый ключ. multimap - потому что одинаковые ключи могут повторяться
// (например, у одного владельца несколько счетов).
// p_getKey - функция (лямбда), которая достаёт ключ из записи
template <class Key, class GetKey>
std::multimap<Key, long> buildIndex(const std::string& p_fileName, GetKey p_getKey)
{
    std::multimap<Key, long> index;

    std::ifstream file(p_fileName.c_str(), std::ios::binary);
    Account account;
    long position = 0;

    while (readAccount(file, account))
    {
        index.insert(std::make_pair(p_getKey(account), position));
        ++position;
    }

    return index;
}

// ищет по индексу все записи с ключом p_key, печатает их на экран
// и сохраняет в новый файл p_resultName, возвращает количество найденных
template <class Key>
int searchAccounts(const std::string& p_fileName, const std::multimap<Key, long>& p_index,
                   const Key& p_key, const std::string& p_resultName)
{
    typedef typename std::multimap<Key, long>::const_iterator Iter;

    std::ifstream file(p_fileName.c_str(), std::ios::binary);
    std::ofstream result(p_resultName.c_str(), std::ios::binary);
    if (!file || !result)
    {
        std::cout << "Can not open files for search" << std::endl;
        return 0;
    }

    // equal_range даёт все элементы индекса с этим ключом
    std::pair<Iter, Iter> range = p_index.equal_range(p_key);

    int count = 0;
    for (Iter it = range.first; it != range.second; ++it)
    {
        // переходим сразу к нужной записи: номер записи * размер записи
        file.seekg(it->second * static_cast<long>(sizeof(Account)));

        Account account;
        if (readAccount(file, account))
        {
            if (count == 0)
                std::cout << tableHeader() << std::endl;

            std::cout << accountToString(account) << std::endl;
            writeAccount(result, account);
            ++count;
        }
    }

    return count;
}

#endif // PART2_H
