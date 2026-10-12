#ifndef MESH_TYPES_H
#define MESH_TYPES_H

#include <iomanip>
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

    // Что это: конструктор по умолчанию (без параметров).
    // Зачем:   чтобы узел можно было создать просто как "Node n;".
    //          Поля сразу получают нули, а не случайный мусор из памяти.
    Node()
        : id(0), x(0.0), y(0.0), z(0.0), isVertex(true)
    {}

    // Что это: конструктор, который создаёт узел с заданным номером и координатами.
    // Зачем:   им пользуются загрузчик, когда читает узлы из файла, и метод
    //          splitEdges, когда добавляет узлы в середины рёбер.
    //          Последний параметр можно не писать - тогда узел считается вершиной.
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

    // Что это: конструктор по умолчанию (без параметров).
    // Зачем:   чтобы элемент можно было создать просто как "FiniteElement fe;".
    //          Номера получают нули, список узлов создаётся пустым.
    FiniteElement()
        : id(0), regionId(0)
    {}

    // Что это: конструктор, который создаёт элемент с номером, номером области
    //          и списком номеров его узлов.
    // Зачем:   им пользуется загрузчик, когда читает элементы из файла.
    FiniteElement(int p_id, int p_regionId, const std::vector<int>& p_nodeIds)
        : id(p_id), regionId(p_regionId), nodeIds(p_nodeIds)
    {}
};

// ------------------------------------------- отформатированный вывод в поток

// Что это: оператор вывода узла в поток (на экран, в файл и т.п.).
// Зачем:   чтобы узел можно было напечатать одной строкой: "std::cout << node;".
//          Выводит номер, три координаты ровными столбиками и тип узла
//          (vertex - вершина, inner - середина ребра). Пример:
//          "Node     1: (  0.000000,   0.000000,   0.000000) vertex"
inline std::ostream& operator<<(std::ostream& p_out, const Node& p_node)
{
    p_out << "Node " << std::setw(5) << p_node.id << ": ("
          << std::fixed << std::setprecision(6)
          << std::setw(10) << p_node.x << ", "
          << std::setw(10) << p_node.y << ", "
          << std::setw(10) << p_node.z << ") "
          << (p_node.isVertex ? "vertex" : "inner");

    return p_out;
}

// Что это: оператор вывода конечного элемента в поток.
// Зачем:   чтобы элемент можно было напечатать одной строкой: "std::cout << fe;".
//          Выводит номер элемента, номер его области и номера всех его узлов.
//          Пример: "FE     1: region =   1, nodes = 13 14 10 79"
inline std::ostream& operator<<(std::ostream& p_out, const FiniteElement& p_element)
{
    p_out << "FE " << std::setw(5) << p_element.id
          << ": region = " << std::setw(3) << p_element.regionId
          << ", nodes =";

    for (size_t i = 0; i < p_element.nodeIds.size(); ++i)
        p_out << " " << p_element.nodeIds[i];

    return p_out;
}

#endif // MESH_TYPES_H
