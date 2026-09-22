#ifndef MESH_TYPES_H
#define MESH_TYPES_H

#include <iostream>
#include <vector>

// Узел сетки
struct Node
{
    int id;         // ID узла (>= 1)
    double x;       // декартовы координаты узла
    double y;
    double z;
    bool isVertex;  // true - узел является вершиной КЭ,
                    // false - узел внутренний (например, середина ребра)

    Node()
        : id(0), x(0.0), y(0.0), z(0.0), isVertex(true)
    {}

    Node(int p_id, double p_x, double p_y, double p_z, bool p_isVertex = true)
        : id(p_id), x(p_x), y(p_y), z(p_z), isVertex(p_isVertex)
    {}
};

// Конечный элемент (тетраэдр для объёмных КЭ или треугольник для граничных)
struct FiniteElement
{
    int id;                     // ID КЭ (>= 1)
    int regionId;               // ID геометрической области (объёма или грани)
    std::vector<int> nodeIds;   // список ID узлов КЭ

    FiniteElement()
        : id(0), regionId(0)
    {}

    FiniteElement(int p_id, int p_regionId, const std::vector<int>& p_nodeIds)
        : id(p_id), regionId(p_regionId), nodeIds(p_nodeIds)
    {}
};

// Отформатированный вывод в поток
std::ostream& operator<<(std::ostream& p_out, const Node& p_node);
std::ostream& operator<<(std::ostream& p_out, const FiniteElement& p_element);

#endif // MESH_TYPES_H
