#ifndef MESH_LOADER_H
#define MESH_LOADER_H

#include "Mesh.h"

#include <string>

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

#endif // MESH_LOADER_H
