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
    // конструктор, запоминает номера трёх узлов, которые будем искать
    HasThreeNodes(int p_id1, int p_id2, int p_id3)
        : m_id1(p_id1), m_id2(p_id2), m_id3(p_id3)
    {}

    // проверяет элемент: true, если в нём есть все три узла
    // вызывается алгоритмом std::find_if для каждого элемента
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
    // конструктор пустой сетки
    Mesh();

    // конструктор сетки из готовых списков узлов и элементов
    Mesh(const std::vector<Node>& p_nodes,
         const std::vector<FiniteElement>& p_elements,
         const std::vector<FiniteElement>& p_boundaryElements);

    // возвращает список всех узлов сетки
    // чтобы снаружи узлы можно было прочитать, но не изменить
    const std::vector<Node>& getNodes() const;

    // возвращает список всех объёмных КЭ (тетраэдров)
    // чтобы снаружи элементы можно было прочитать, но не изменить
    const std::vector<FiniteElement>& getElements() const;

    // возвращает список всех граничных КЭ (треугольников на поверхности)
    // чтобы снаружи граничные элементы можно было прочитать, но не изменить
    const std::vector<FiniteElement>& getBoundaryElements() const;

    // возвращает узел по его номеру (номера начинаются с 1)
    // если такого узла нет, бросает исключение BadIdException
    const Node& getNode(int p_id) const;

    // ищет все тетраэдры, в которых есть три заданных узла
    // (три узла - это треугольная грань, ищем тетраэдры с этой гранью)
    std::vector<int> findElementsByThreeNodes(int p_id1, int p_id2, int p_id3) const;

    // ищет все тетраэдры, у которых есть ребро между двумя заданными узлами
    std::vector<int> findElementsByEdge(int p_id1, int p_id2) const;

    // возвращает номера всех узлов на заданной границе (грани детали)
    // к этим узлам при расчёте прикладывают закрепление или нагрузку
    std::vector<int> getBoundaryNodeIds(int p_boundaryId) const;

    // возвращает номера всех тетраэдров с заданным номером области
    // (например, все элементы из одного материала)
    std::vector<int> getElementIdsByRegion(int p_regionId) const;

    // возвращает номера всех граничных треугольников на заданной границе
    std::vector<int> getBoundaryElementIds(int p_boundaryId) const;

    // для каждого узла находит соседей - узлы, соединённые с ним ребром
    // соседи узла с номером id лежат в результате по индексу id - 1
    std::vector<std::vector<int> > getNodeNeighbours() const;

    // добавляет новый узел в середину каждого ребра всех элементов
    // тетраэдр становится 10-узловым, треугольник - 6-узловым
    void splitEdges();

    // печатает узел в заданный поток (экран, файл) и переводит строку
    void printNode(std::ostream& p_out, const Node& p_node) const;

    // печатает элемент в заданный поток (экран, файл) и переводит строку
    void printElement(std::ostream& p_out, const FiniteElement& p_element) const;

private:
    // Число вершин у КЭ: 4 у тетраэдра, 3 у треугольника
    static const int VERTICES_IN_ELEMENT = 4;
    static const int VERTICES_IN_BOUNDARY_ELEMENT = 3;

    // составляет список рёбер элемента по числу его вершин
    // у тетраэдра (4 вершины) получается 6 рёбер, у треугольника (3 вершины) - 3
    static std::vector<std::pair<int, int> > makeEdges(int p_vertexCount);

    // вспомогательная часть splitEdges: добавляет узлы в середины рёбер
    // элементов одного списка, p_createdNodes помнит, на каких рёбрах узел уже есть
    void addMiddleNodes(std::vector<FiniteElement>& p_elements,
                        int p_vertexCount,
                        std::map<std::pair<int, int>, int>& p_createdNodes);

    std::vector<Node> m_nodes;
    std::vector<FiniteElement> m_elements;          // тетраэдры
    std::vector<FiniteElement> m_boundaryElements;  // треугольники
};

#endif // MESH_H
