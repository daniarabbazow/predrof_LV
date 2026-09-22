#ifndef MESH_EXCEPTIONS_H
#define MESH_EXCEPTIONS_H

#include <stdexcept>
#include <string>

// Базовое исключение загрузчика сетки
class MeshException : public std::runtime_error
{
public:
    explicit MeshException(const std::string& p_message)
        : std::runtime_error(p_message)
    {}
};

// Не удалось открыть файл сетки
class FileOpenException : public MeshException
{
public:
    explicit FileOpenException(const std::string& p_fileName)
        : MeshException("Can not open mesh file: " + p_fileName)
    {}
};

// Файл открылся, но его содержимое не соответствует формату
class FileFormatException : public MeshException
{
public:
    explicit FileFormatException(const std::string& p_message)
        : MeshException("Wrong mesh file format: " + p_message)
    {}
};

// Обращение к узлу или элементу с недопустимым ID
class BadIdException : public MeshException
{
public:
    explicit BadIdException(const std::string& p_message)
        : MeshException("Bad ID: " + p_message)
    {}
};

#endif // MESH_EXCEPTIONS_H
