#ifndef MESH_EXCEPTIONS_H
#define MESH_EXCEPTIONS_H

#include <stdexcept>
#include <string>

// Базовое исключение загрузчика сетки
class MeshException : public std::runtime_error
{
public:
    // Что это: конструктор исключения, принимает текст ошибки.
    // Зачем:   передаёт текст в std::runtime_error, который его хранит.
    //          Потом этот текст можно получить через e.what() в блоке catch.
    explicit MeshException(const std::string& p_message)
        : std::runtime_error(p_message)
    {}
};

// Не удалось открыть файл сетки
class FileOpenException : public MeshException
{
public:
    // Что это: конструктор, принимает имя файла, который не удалось открыть.
    // Зачем:   сам собирает понятное сообщение "Can not open mesh file: <имя>",
    //          поэтому в месте ошибки достаточно написать throw FileOpenException(имя).
    explicit FileOpenException(const std::string& p_fileName)
        : MeshException("Can not open mesh file: " + p_fileName)
    {}
};

// Файл открылся, но его содержимое не соответствует формату
class FileFormatException : public MeshException
{
public:
    // Что это: конструктор, принимает описание того, что не так с файлом.
    // Зачем:   добавляет к описанию начало "Wrong mesh file format: ",
    //          чтобы по сообщению сразу было видно, что ошибка в содержимом файла.
    explicit FileFormatException(const std::string& p_message)
        : MeshException("Wrong mesh file format: " + p_message)
    {}
};

// Обращение к узлу или элементу с недопустимым ID
class BadIdException : public MeshException
{
public:
    // Что это: конструктор, принимает описание неверного номера.
    // Зачем:   добавляет к описанию начало "Bad ID: ", чтобы было видно,
    //          что программа обратилась к узлу, которого нет.
    explicit BadIdException(const std::string& p_message)
        : MeshException("Bad ID: " + p_message)
    {}
};

#endif // MESH_EXCEPTIONS_H
