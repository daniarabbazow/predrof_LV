#include "MeshLoader.h"
#include "MeshExceptions.h"

#include <sstream>

// readHeader: чтение заголовка блока.
// Как работает: читает два числа. Если прочитать не удалось или числа
// не положительные, бросает FileFormatException с названием блока,
// чтобы по сообщению было понятно, в каком месте файла ошибка.
void AneuMeshLoader::readHeader(std::ifstream& p_file, int& o_count, int& o_size,
                                const std::string& p_blockName)
{
    p_file >> o_count >> o_size;

    if (!p_file)
        throw FileFormatException("can not read header of block \"" + p_blockName + "\"");

    if (o_count <= 0 || o_size <= 0)
        throw FileFormatException("wrong header of block \"" + p_blockName + "\"");
}

// readElements: чтение блока элементов.
// Как работает: для каждой из p_count строк читает номер области и
// p_nodesInElement номеров узлов. Каждый номер узла проверяется: он должен быть
// от 1 до количества узлов в сетке. Номер самого элемента - это номер строки
// в блоке (первая строка - элемент 1). Если что-то не прочиталось - исключение.
std::vector<FiniteElement> AneuMeshLoader::readElements(std::ifstream& p_file,
                                                        int p_count,
                                                        int p_nodesInElement,
                                                        int p_nodesInMesh,
                                                        const std::string& p_blockName)
{
    std::vector<FiniteElement> elements;
    elements.reserve(p_count);

    for (int i = 0; i < p_count; ++i)
    {
        int regionId = 0;
        p_file >> regionId;

        std::vector<int> nodeIds(p_nodesInElement);

        for (int j = 0; j < p_nodesInElement; ++j)
        {
            p_file >> nodeIds[j];

            // ID узла должен указывать на существующий узел сетки
            if (p_file && (nodeIds[j] < 1 || nodeIds[j] > p_nodesInMesh))
            {
                std::ostringstream message;
                message << "block \"" << p_blockName << "\", element " << (i + 1)
                        << " refers to node " << nodeIds[j];
                throw FileFormatException(message.str());
            }
        }

        if (!p_file)
        {
            std::ostringstream message;
            message << "can not read element " << (i + 1)
                    << " of block \"" << p_blockName << "\"";
            throw FileFormatException(message.str());
        }

        // ID элемента - его порядковый номер в файле, начиная с 1
        elements.push_back(FiniteElement(i + 1, regionId, nodeIds));
    }

    return elements;
}

// loadMesh: загрузка сетки из файла .aneu.
// Как работает:
//   1) открывает файл, если не получилось - бросает FileOpenException;
//   2) читает заголовок и все узлы (номер узла - это номер его строки, с 1);
//   3) читает заголовок и блок тетраэдров (в каждом должно быть 4 узла);
//   4) читает заголовок и блок граничных треугольников (в каждом 3 узла);
//   5) собирает из прочитанного объект Mesh и возвращает его.
// На каждом шаге проверяет, что данные прочитались, иначе - FileFormatException.
Mesh AneuMeshLoader::loadMesh(const std::string& p_fileName)
{
    std::ifstream file(p_fileName.c_str());

    if (!file.is_open())
        throw FileOpenException(p_fileName);

    // ---- блок узлов ----
    int nodesCount = 0;
    int dimension = 0;
    readHeader(file, nodesCount, dimension, "nodes");

    if (dimension != SPACE_DIMENSION)
        throw FileFormatException("only 3D meshes are supported");

    std::vector<Node> nodes;
    nodes.reserve(nodesCount);

    for (int i = 0; i < nodesCount; ++i)
    {
        double x = 0.0;
        double y = 0.0;
        double z = 0.0;

        file >> x >> y >> z;

        if (!file)
        {
            std::ostringstream message;
            message << "can not read node " << (i + 1);
            throw FileFormatException(message.str());
        }

        // Все узлы из файла являются вершинами КЭ
        nodes.push_back(Node(i + 1, x, y, z, true));
    }

    // ---- блок объёмных КЭ ----
    int elementsCount = 0;
    int nodesInElement = 0;
    readHeader(file, elementsCount, nodesInElement, "finite elements");

    if (nodesInElement != NODES_IN_ELEMENT)
        throw FileFormatException("only tetrahedral finite elements are supported");

    std::vector<FiniteElement> elements =
        readElements(file, elementsCount, nodesInElement, nodesCount, "finite elements");

    // ---- блок граничных КЭ ----
    int boundaryCount = 0;
    int nodesInBoundaryElement = 0;
    readHeader(file, boundaryCount, nodesInBoundaryElement, "boundary elements");

    if (nodesInBoundaryElement != NODES_IN_BOUNDARY_ELEMENT)
        throw FileFormatException("only triangular boundary elements are supported");

    std::vector<FiniteElement> boundaryElements =
        readElements(file, boundaryCount, nodesInBoundaryElement, nodesCount,
                     "boundary elements");

    return Mesh(nodes, elements, boundaryElements);
}
