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
    // Что это: виртуальный деструктор.
    // Зачем:   если удалить загрузчик-наследник через указатель на MeshLoader
    //          (delete loader;), то без virtual вызвался бы только деструктор
    //          MeshLoader, а деструктор наследника - нет.
    virtual ~MeshLoader() {}

    // Что это: загрузка сетки из файла. "= 0" значит, что у метода здесь нет
    //          тела - его обязан написать каждый класс-наследник.
    // Зачем:   это общий "договор" для всех загрузчиков: любой из них умеет
    //          loadMesh(имя файла) и возвращает готовую сетку Mesh. Для нового
    //          формата файла достаточно написать нового наследника.
    //          При ошибке метод бросает исключение, производное от MeshException.
    virtual Mesh loadMesh(const std::string& p_fileName) = 0;
};

// Загрузчик сетки из файла формата .aneu
class AneuMeshLoader : public MeshLoader
{
public:
    // Что это: загрузка сетки из файла формата .aneu. Слово override значит,
    //          что это переопределение метода loadMesh из класса MeshLoader.
    // Зачем:   открывает файл, по очереди читает узлы, тетраэдры и граничные
    //          треугольники и собирает из них объект Mesh. Если файла нет
    //          или он испорчен, бросает исключение.
    Mesh loadMesh(const std::string& p_fileName) override;

private:
    // Что это: читает строку-заголовок блока - два числа: сколько строк в блоке
    //          и сколько чисел в каждой строке (например, "85 3").
    // Зачем:   в файле три блока (узлы, тетраэдры, треугольники), и у каждого есть
    //          такой заголовок. Чтобы не писать одно и то же чтение с проверками
    //          трижды, оно вынесено в отдельный метод. Прочитанные числа
    //          записываются в o_count и o_size.
    void readHeader(std::ifstream& p_file, int& o_count, int& o_size,
                    const std::string& p_blockName);

    // Что это: читает блок конечных элементов - p_count строк вида
    //          "<номер области> <номер узла> <номер узла> ...".
    // Зачем:   тетраэдры и граничные треугольники записаны в файле одинаково,
    //          отличается только число узлов в строке. Поэтому один метод читает
    //          оба блока. Заодно проверяет, что все номера узлов существуют.
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
