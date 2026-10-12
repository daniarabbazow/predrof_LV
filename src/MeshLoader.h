#ifndef MESH_LOADER_H
#define MESH_LOADER_H

#include "Mesh.h"

#include <fstream>
#include <string>
#include <vector>

// Базовый класс загрузчика сетки.
// Для поддержки нового формата файла достаточно создать
// производный класс и переопределить в нём метод loadMesh.
class MeshLoader
{
public:
    // виртуальный деструктор: чтобы при удалении через указатель на MeshLoader
    // вызывался и деструктор наследника
    virtual ~MeshLoader() {}

    // загружает сетку из файла, "= 0" - этот метод обязан написать каждый наследник
    // при ошибке бросает исключение, производное от MeshException
    virtual Mesh loadMesh(const std::string& p_fileName) = 0;
};

// Загрузчик сетки из файла формата .aneu
class AneuMeshLoader : public MeshLoader
{
public:
    // загружает сетку из файла формата .aneu
    // (override - это переопределение метода loadMesh из MeshLoader)
    Mesh loadMesh(const std::string& p_fileName) override;

private:
    // читает заголовок блока: сколько строк в блоке и сколько чисел в строке
    // результат записывается в o_count и o_size
    void readHeader(std::ifstream& p_file, int& o_count, int& o_size,
                    const std::string& p_blockName);

    // читает блок элементов (тетраэдры или граничные треугольники)
    // и проверяет, что все номера узлов существуют
    std::vector<FiniteElement> readElements(std::ifstream& p_file,
                                            int p_count,
                                            int p_nodesInElement,
                                            int p_nodesInMesh,
                                            const std::string& p_blockName);

    static const int SPACE_DIMENSION = 3;            // размерность пространства
    static const int NODES_IN_ELEMENT = 4;           // узлов в тетраэдре
    static const int NODES_IN_BOUNDARY_ELEMENT = 3;  // узлов в треугольнике
};

#endif // MESH_LOADER_H
