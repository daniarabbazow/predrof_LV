#ifndef MESH_H
#define MESH_H

#include "MeshTypes.h"

#include <iostream>
#include <map>
#include <utility>
#include <vector>

// Предикат: КЭ содержит все три заданных узла.
// Используется вместе с алгоритмом std::find_if.
class HasThreeNodes
{
public:
    HasThreeNodes(int p_id1, int p_id2, int p_id3)
        : m_id1(p_id1), m_id2(p_id2), m_id3(p_id3)
    {}

    bool operator()(const FiniteElement& p_element) const;

private:
    int m_id1;
    int m_id2;
    int m_id3;
};

// Класс сетки. Хранит узлы, объёмные КЭ (тетраэдры)
// и граничные КЭ (треугольники).
class Mesh
{
public:
    Mesh();
    Mesh(const std::vector<Node>& p_nodes,
         const std::vector<FiniteElement>& p_elements,
         const std::vector<FiniteElement>& p_boundaryElements);

    // Доступ к контейнерам сетки
    const std::vector<Node>& getNodes() const;
    const std::vector<FiniteElement>& getElements() const;
    const std::vector<FiniteElement>& getBoundaryElements() const;

    // Узел по его ID (ID >= 1)
    const Node& getNode(int p_id) const;

    // ID тетраэдров, содержащих три заданных узла
    std::vector<int> findElementsByThreeNodes(int p_id1, int p_id2, int p_id3) const;

    // ID тетраэдров с общим ребром, заданным двумя узлами
    std::vector<int> findElementsByEdge(int p_id1, int p_id2) const;

    // ID граничных узлов, лежащих на заданной границе
    std::vector<int> getBoundaryNodeIds(int p_boundaryId) const;

    // ID тетраэдров, лежащих в заданной области
    std::vector<int> getElementIdsByRegion(int p_regionId) const;

    // ID граничных КЭ, лежащих на заданной границе
    std::vector<int> getBoundaryElementIds(int p_boundaryId) const;

    // Соседние по рёбрам узлы. Элемент с индексом (id - 1)
    // хранит ID всех узлов, соединённых ребром с узлом id.
    std::vector<std::vector<int> > getNodeNeighbours() const;

    // Вставка новых узлов в середины рёбер всех КЭ сетки
    void splitEdges();

    // Отформатированный вывод в заданный поток
    void printNode(std::ostream& p_out, const Node& p_node) const;
    void printElement(std::ostream& p_out, const FiniteElement& p_element) const;

private:
    // Число вершин у КЭ: 4 у тетраэдра, 3 у треугольника
    static const int VERTICES_IN_ELEMENT = 4;
    static const int VERTICES_IN_BOUNDARY_ELEMENT = 3;

    // Список рёбер КЭ в виде пар индексов в списке узлов элемента
    static std::vector<std::pair<int, int> > makeEdges(int p_vertexCount);

    // Добавление середин рёбер в элементы контейнера p_elements.
    // Общие для нескольких КЭ рёбра обрабатываются один раз
    // (для этого используется словарь уже созданных узлов p_createdNodes).
    void addMiddleNodes(std::vector<FiniteElement>& p_elements,
                        int p_vertexCount,
                        std::map<std::pair<int, int>, int>& p_createdNodes);

    std::vector<Node> m_nodes;
    std::vector<FiniteElement> m_elements;          // тетраэдры
    std::vector<FiniteElement> m_boundaryElements;  // треугольники
};

#endif // MESH_H
