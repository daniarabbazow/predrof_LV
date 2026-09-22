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
    virtual ~MeshLoader() {}

    // Загружает сетку из файла p_fileName.
    // При ошибке генерирует исключение, производное от MeshException.
    virtual Mesh loadMesh(const std::string& p_fileName) = 0;
};

// Загрузчик сетки из файла формата .aneu
class AneuMeshLoader : public MeshLoader
{
public:
    Mesh loadMesh(const std::string& p_fileName) override;

private:
    // Читает заголовок очередного блока: количество строк и
    // количество чисел (координат или узлов) в одной строке
    void readHeader(std::ifstream& p_file, int& o_count, int& o_size,
                    const std::string& p_blockName);

    // Читает блок конечных элементов (объёмных или граничных)
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
