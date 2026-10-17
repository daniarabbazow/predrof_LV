#include "Part2.h"
#include "Input.h"

#include <cstdio>
#include <cstring>
#include <iomanip>
#include <sstream>

// ширина строки таблицы без колонки дохода
const int TABLE_WIDTH = 75;

// ------------------------------------------------- чтение и запись записей

// read читает ровно sizeof(Account) байт прямо в структуру,
// поток в условии становится false, если прочитать не удалось
bool readAccount(std::istream& p_file, Account& o_account)
{
    return static_cast<bool>(p_file.read(reinterpret_cast<char*>(&o_account), sizeof(Account)));
}

// write записывает все байты структуры одним вызовом
void writeAccount(std::ostream& p_file, const Account& p_account)
{
    p_file.write(reinterpret_cast<const char*>(&p_account), sizeof(Account));
}

// ------------------------------------------------------------------- ввод

// по очереди спрашиваем каждое поле, фамилию копируем в массив char
Account inputAccount()
{
    Account account = Account();   // все поля сначала заполняются нулями

    account.number = readInt("Account number: ", 1, 999999999);
    account.code = readInt("Account code: ", 0, 999999);

    std::string owner = readWord("Owner surname (latin letters): ");
    std::strncpy(account.owner, owner.c_str(), OWNER_SIZE - 1);

    account.amount = readDouble("Amount: ", 0, 1e12);
    account.openDate.day = readInt("Open date - day: ", 1, 31);
    account.openDate.month = readInt("Open date - month: ", 1, 12);
    account.openDate.year = readInt("Open date - year: ", 1900, 2100);
    account.percent = readDouble("Annual percent: ", 0, 100);

    return account;
}

// файл открывается в режиме app, поэтому новые записи дописываются в конец;
// счёт с уже существующим номером не добавляется
void addAccounts(const std::string& p_fileName)
{
    std::multimap<int, long> numbers = buildNumberIndex(p_fileName);

    int count = readInt("How many accounts to add: ", 1, 100);

    std::ofstream file(p_fileName.c_str(), std::ios::binary | std::ios::app);
    if (!file)
    {
        std::cout << "Can not open file " << p_fileName << std::endl;
        return;
    }

    for (int i = 0; i < count; ++i)
    {
        std::cout << "--- account " << i + 1 << " of " << count << " ---" << std::endl;
        Account account = inputAccount();

        if (numbers.count(account.number) > 0)
        {
            std::cout << "Account " << account.number << " already exists, skipped" << std::endl;
            continue;
        }

        writeAccount(file, account);
        numbers.insert(std::make_pair(account.number, 0L));   // тут важен только номер
    }
}

// ------------------------------------------------------------------- вывод

// заголовок с теми же ширинами колонок, что и в accountToString
std::string tableHeader()
{
    std::ostringstream line;
    line << std::setw(10) << "Number" << std::setw(8) << "Code" << "  "
         << std::left << std::setw(20) << "Owner" << std::right
         << std::setw(12) << "Opened" << std::setw(14) << "Amount"
         << std::setw(9) << "Percent";
    return line.str();
}

// строку собираем в ostringstream, чтобы настройки вывода (fixed, setprecision)
// не остались у std::cout
std::string accountToString(const Account& p_account)
{
    std::ostringstream date;
    date << std::setfill('0') << std::setw(2) << p_account.openDate.day << '.'
         << std::setw(2) << p_account.openDate.month << '.'
         << std::setw(4) << p_account.openDate.year;

    std::ostringstream line;
    line << std::setw(10) << p_account.number << std::setw(8) << p_account.code << "  "
         << std::left << std::setw(20) << p_account.owner << std::right
         << std::setw(12) << date.str()
         << std::fixed << std::setprecision(2)
         << std::setw(14) << p_account.amount << std::setw(9) << p_account.percent;
    return line.str();
}

// читаем записи по одной и сразу печатаем
int showAccounts(const std::string& p_fileName)
{
    std::ifstream file(p_fileName.c_str(), std::ios::binary);

    int count = 0;
    Account account;
    while (readAccount(file, account))
    {
        if (count == 0)
            std::cout << tableHeader() << std::endl;

        std::cout << accountToString(account) << std::endl;
        ++count;
    }

    if (count == 0)
        std::cout << "No accounts in " << p_fileName << std::endl;

    return count;
}

// ---------------------------------------------------- удаление и замена

// переписываем все записи во временный файл, кроме удаляемой,
// потом старый файл удаляем, а временный переименовываем в старое имя
bool deleteAccount(const std::string& p_fileName, int p_number)
{
    std::ifstream source(p_fileName.c_str(), std::ios::binary);
    std::ofstream target(TEMP_FILE.c_str(), std::ios::binary);
    if (!source || !target)
    {
        std::cout << "Can not open files" << std::endl;
        return false;
    }

    bool found = false;
    Account account;
    while (readAccount(source, account))
    {
        if (account.number == p_number)
            found = true;                    // эту запись не переписываем
        else
            writeAccount(target, account);
    }

    source.close();
    target.close();

    if (!found)
    {
        std::remove(TEMP_FILE.c_str());
        return false;
    }

    std::remove(p_fileName.c_str());
    std::rename(TEMP_FILE.c_str(), p_fileName.c_str());
    return true;
}

// так же, как deleteAccount, только вместо найденной записи пишем новую
bool replaceAccount(const std::string& p_fileName, int p_number, const Account& p_newAccount)
{
    std::ifstream source(p_fileName.c_str(), std::ios::binary);
    std::ofstream target(TEMP_FILE.c_str(), std::ios::binary);
    if (!source || !target)
    {
        std::cout << "Can not open files" << std::endl;
        return false;
    }

    bool found = false;
    Account account;
    while (readAccount(source, account))
    {
        if (account.number == p_number)
        {
            writeAccount(target, p_newAccount);
            found = true;
        }
        else
        {
            writeAccount(target, account);
        }
    }

    source.close();
    target.close();

    if (!found)
    {
        std::remove(TEMP_FILE.c_str());
        return false;
    }

    std::remove(p_fileName.c_str());
    std::rename(TEMP_FILE.c_str(), p_fileName.c_str());
    return true;
}

// ---------------------------------------------------------------- ведомость

// каждая запись - строка таблицы плюс доход за год (сумма * процент / 100),
// в конце итоговая строка: число счетов, общая сумма и общий доход
void makeReport(const std::string& p_binaryName, const std::string& p_textName)
{
    std::ifstream source(p_binaryName.c_str(), std::ios::binary);
    std::ofstream report(p_textName.c_str());
    if (!source || !report)
    {
        std::cout << "Can not open files" << std::endl;
        return;
    }

    const std::string separator(TABLE_WIDTH + 14, '-');

    report << "BANK ACCOUNTS REPORT" << "\n\n";
    report << tableHeader() << std::setw(14) << "Income/year" << "\n";
    report << separator << "\n";

    report << std::fixed << std::setprecision(2);

    int count = 0;
    double totalAmount = 0.0;
    double totalIncome = 0.0;

    Account account;
    while (readAccount(source, account))
    {
        double income = account.amount * account.percent / 100.0;

        report << accountToString(account) << std::setw(14) << income << "\n";

        totalAmount += account.amount;
        totalIncome += income;
        ++count;
    }

    std::ostringstream totalTitle;
    totalTitle << "TOTAL (" << count << " accounts)";

    report << separator << "\n";
    report << std::left << std::setw(TABLE_WIDTH - 23) << totalTitle.str() << std::right
           << std::setw(14) << totalAmount << std::setw(9) << "" << std::setw(14) << totalIncome
           << "\n";
}

// ------------------------------------------------------------------ индексы

// индекс по номеру счёта
std::multimap<int, long> buildNumberIndex(const std::string& p_fileName)
{
    return buildIndex<int>(p_fileName,
        [](const Account& p_account) { return p_account.number; });
}

// индекс по коду счёта
std::multimap<int, long> buildCodeIndex(const std::string& p_fileName)
{
    return buildIndex<int>(p_fileName,
        [](const Account& p_account) { return p_account.code; });
}

// индекс по фамилии владельца
std::multimap<std::string, long> buildOwnerIndex(const std::string& p_fileName)
{
    return buildIndex<std::string>(p_fileName,
        [](const Account& p_account) { return std::string(p_account.owner); });
}

// --------------------------------------------------------------------- меню

// печатает итог поиска
void printSearchResult(int p_count)
{
    if (p_count == 0)
        std::cout << "Nothing found" << std::endl;
    else
        std::cout << "Found: " << p_count << ", saved to " << SEARCH_FILE << std::endl;
}

// меню части 2: каждый пункт вызывает одну из функций выше
void part2Menu()
{
    while (true)
    {
        std::cout << std::endl
                  << "--- Part 2: bank accounts (" << ACCOUNTS_FILE << ") ---" << std::endl
                  << "1. Add accounts from keyboard" << std::endl
                  << "2. Show all accounts" << std::endl
                  << "3. Delete account by number" << std::endl
                  << "4. Replace account by number" << std::endl
                  << "5. Search by account number" << std::endl
                  << "6. Search by account code" << std::endl
                  << "7. Search by owner" << std::endl
                  << "8. Show last search result (" << SEARCH_FILE << ")" << std::endl
                  << "9. Make report (" << REPORT_FILE << ")" << std::endl
                  << "0. Back" << std::endl;

        int choice = readInt("Your choice: ", 0, 9);

        switch (choice)
        {
        case 0:
            return;

        case 1:
            addAccounts(ACCOUNTS_FILE);
            break;

        case 2:
            showAccounts(ACCOUNTS_FILE);
            break;

        case 3:
        {
            int number = readInt("Number of account to delete: ", 1, 999999999);
            if (deleteAccount(ACCOUNTS_FILE, number))
                std::cout << "Account " << number << " deleted" << std::endl;
            else
                std::cout << "No account with number " << number << std::endl;
            break;
        }

        case 4:
        {
            int number = readInt("Number of account to replace: ", 1, 999999999);
            std::multimap<int, long> numbers = buildNumberIndex(ACCOUNTS_FILE);
            if (numbers.count(number) == 0)
            {
                std::cout << "No account with number " << number << std::endl;
                break;
            }

            std::cout << "Enter new data:" << std::endl;
            Account account = inputAccount();

            // новый номер не должен совпадать с номером другого счёта
            if (account.number != number && numbers.count(account.number) > 0)
            {
                std::cout << "Account " << account.number << " already exists" << std::endl;
                break;
            }

            replaceAccount(ACCOUNTS_FILE, number, account);
            std::cout << "Account " << number << " replaced" << std::endl;
            break;
        }

        case 5:
        {
            int number = readInt("Account number: ", 1, 999999999);
            printSearchResult(searchAccounts(ACCOUNTS_FILE, buildNumberIndex(ACCOUNTS_FILE),
                                             number, SEARCH_FILE));
            break;
        }

        case 6:
        {
            int code = readInt("Account code: ", 0, 999999);
            printSearchResult(searchAccounts(ACCOUNTS_FILE, buildCodeIndex(ACCOUNTS_FILE),
                                             code, SEARCH_FILE));
            break;
        }

        case 7:
        {
            std::string owner = readWord("Owner surname: ");
            printSearchResult(searchAccounts(ACCOUNTS_FILE, buildOwnerIndex(ACCOUNTS_FILE),
                                             owner, SEARCH_FILE));
            break;
        }

        case 8:
            showAccounts(SEARCH_FILE);
            break;

        case 9:
            makeReport(ACCOUNTS_FILE, REPORT_FILE);
            std::cout << "Report saved to " << REPORT_FILE << std::endl;
            break;
        }
    }
}
