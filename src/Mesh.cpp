#include "Mesh.h"
#include "MeshExceptions.h"

#include <algorithm>
#include <set>
#include <sstream>

// ---------------------------------------------------------------- предикат

bool HasThreeNodes::operator()(const FiniteElement& p_element) const
{
    const std::vector<int>& nodes = p_element.nodeIds;

    // КЭ подходит, если в его списке узлов есть все три заданных ID
    return std::find(nodes.begin(), nodes.end(), m_id1) != nodes.end()
        && std::find(nodes.begin(), nodes.end(), m_id2) != nodes.end()
        && std::find(nodes.begin(), nodes.end(), m_id3) != nodes.end();
}

// ------------------------------------------------------------ конструкторы

Mesh::Mesh()
{}

Mesh::Mesh(const std::vector<Node>& p_nodes,
           const std::vector<FiniteElement>& p_elements,
           const std::vector<FiniteElement>& p_boundaryElements)
    : m_nodes(p_nodes)
    , m_elements(p_elements)
    , m_boundaryElements(p_boundaryElements)
{}

// ------------------------------------------------------- доступ к данным

const std::vector<Node>& Mesh::getNodes() const
{
    return m_nodes;
}

const std::vector<FiniteElement>& Mesh::getElements() const
{
    return m_elements;
}

const std::vector<FiniteElement>& Mesh::getBoundaryElements() const
{
    return m_boundaryElements;
}

const Node& Mesh::getNode(int p_id) const
{
    // Узлы хранятся подряд, поэтому узел с ID = n лежит по индексу n - 1.
    // Доступ выполняется за O(1).
    if (p_id < 1 || p_id > static_cast<int>(m_nodes.size()))
    {
        std::ostringstream message;
        message << "node " << p_id << " does not exist";
        throw BadIdException(message.str());
    }

    return m_nodes[p_id - 1];
}

// ------------------------------------------------------- поиск элементов

std::vector<int> Mesh::findElementsByThreeNodes(int p_id1, int p_id2, int p_id3) const
{
    std::vector<int> result;
    HasThreeNodes predicate(p_id1, p_id2, p_id3);

    // Ищем все подходящие КЭ: каждый следующий поиск
    // начинается со следующей за найденным КЭ позиции
    std::vector<FiniteElement>::const_iterator it = m_elements.begin();

    while ((it = std::find_if(it, m_elements.end(), predicate)) != m_elements.end())
    {
        result.push_back(it->id);
        ++it;
    }

    return result;
}

std::vector<int> Mesh::findElementsByEdge(int p_id1, int p_id2) const
{
    std::vector<int> result;

    // Ребро задано двумя узлами, поэтому достаточно проверить,
    // что оба узла входят в список узлов КЭ
    std::for_each(m_elements.begin(), m_elements.end(),
        [&](const FiniteElement& p_element)
        {
            const std::vector<int>& nodes = p_element.nodeIds;

            if (std::find(nodes.begin(), nodes.end(), p_id1) != nodes.end()
             && std::find(nodes.begin(), nodes.end(), p_id2) != nodes.end())
                result.push_back(p_element.id);
        });

    return result;
}

std::vector<int> Mesh::getElementIdsByRegion(int p_regionId) const
{
    std::vector<int> result;

    std::for_each(m_elements.begin(), m_elements.end(),
        [&](const FiniteElement& p_element)
        {
            if (p_element.regionId == p_regionId)
                result.push_back(p_element.id);
        });

    return result;
}

std::vector<int> Mesh::getBoundaryElementIds(int p_boundaryId) const
{
    std::vector<int> result;

    std::for_each(m_boundaryElements.begin(), m_boundaryElements.end(),
        [&](const FiniteElement& p_element)
        {
            if (p_element.regionId == p_boundaryId)
                result.push_back(p_element.id);
        });

    return result;
}

std::vector<int> Mesh::getBoundaryNodeIds(int p_boundaryId) const
{
    // Один узел принадлежит нескольким граничным КЭ,
    // поэтому повторы убираем с помощью множества
    std::set<int> nodes;

    std::for_each(m_boundaryElements.begin(), m_boundaryElements.end(),
        [&](const FiniteElement& p_element)
        {
            if (p_element.regionId == p_boundaryId)
                nodes.insert(p_element.nodeIds.begin(), p_element.nodeIds.end());
        });

    return std::vector<int>(nodes.begin(), nodes.end());
}

// ------------------------------------------------------- соседние узлы

std::vector<std::pair<int, int> > Mesh::makeEdges(int p_vertexCount)
{
    // Все вершины КЭ соединены рёбрами попарно:
    // у тетраэдра получается 6 рёбер, у треугольника - 3
    std::vector<std::pair<int, int> > edges;

    for (int i = 0; i < p_vertexCount; ++i)
        for (int j = i + 1; j < p_vertexCount; ++j)
            edges.push_back(std::make_pair(i, j));

    return edges;
}

std::vector<std::vector<int> > Mesh::getNodeNeighbours() const
{
    // Для каждого узла собираем множество соседей (без повторов)
    std::vector<std::set<int> > neighbours(m_nodes.size());

    // Лямбда обрабатывает один КЭ: добавляет соседей по всем его рёбрам
    auto processElement = [&](const FiniteElement& p_element, int p_vertexCount)
    {
        std::vector<std::pair<int, int> > edges = makeEdges(p_vertexCount);

        std::for_each(edges.begin(), edges.end(),
            [&](const std::pair<int, int>& p_edge)
            {
                int first = p_element.nodeIds[p_edge.first];
                int second = p_element.nodeIds[p_edge.second];

                neighbours[first - 1].insert(second);
                neighbours[second - 1].insert(first);
            });
    };

    std::for_each(m_elements.begin(), m_elements.end(),
        [&](const FiniteElement& p_element)
        {
            processElement(p_element, VERTICES_IN_ELEMENT);
        });

    std::for_each(m_boundaryElements.begin(), m_boundaryElements.end(),
        [&](const FiniteElement& p_element)
        {
            processElement(p_element, VERTICES_IN_BOUNDARY_ELEMENT);
        });

    // Переводим множества в контейнеры ID
    std::vector<std::vector<int> > result(neighbours.size());

    std::transform(neighbours.begin(), neighbours.end(), result.begin(),
        [](const std::set<int>& p_nodes)
        {
            return std::vector<int>(p_nodes.begin(), p_nodes.end());
        });

    return result;
}

// --------------------------------------------- вставка узлов в середины рёбер

void Mesh::addMiddleNodes(std::vector<FiniteElement>& p_elements,
                          int p_vertexCount,
                          std::map<std::pair<int, int>, int>& p_createdNodes)
{
    std::vector<std::pair<int, int> > edges = makeEdges(p_vertexCount);

    std::for_each(p_elements.begin(), p_elements.end(),
        [&](FiniteElement& p_element)
        {
            std::for_each(edges.begin(), edges.end(),
                [&](const std::pair<int, int>& p_edge)
                {
                    int first = p_element.nodeIds[p_edge.first];
                    int second = p_element.nodeIds[p_edge.second];

                    // Ребро задаём парой ID, где первый ID меньше второго.
                    // Тогда одно и то же ребро разных КЭ даёт один и тот же ключ.
                    std::pair<int, int> key = std::make_pair(std::min(first, second),
                                                             std::max(first, second));

                    std::map<std::pair<int, int>, int>::const_iterator found =
                        p_createdNodes.find(key);

                    if (found == p_createdNodes.end())
                    {
                        // Узел на этом ребре ещё не создан - создаём его.
                        // Координаты копируем, так как после push_back
                        // ссылки на элементы вектора становятся недействительными.
                        Node a = getNode(first);
                        Node b = getNode(second);

                        int newId = static_cast<int>(m_nodes.size()) + 1;

                        m_nodes.push_back(Node(newId,
                                               (a.x + b.x) / 2.0,
                                               (a.y + b.y) / 2.0,
                                               (a.z + b.z) / 2.0,
                                               false));

                        found = p_createdNodes.insert(std::make_pair(key, newId)).first;
                    }

                    // Новый узел дописываем в конец списка узлов КЭ
                    // в том же порядке, в котором перечислены рёбра
                    p_element.nodeIds.push_back(found->second);
                });
        });
}

void Mesh::splitEdges()
{
    // Словарь "ребро -> ID созданного на нём узла" общий для объёмных
    // и граничных КЭ, поэтому на общем ребре узел создаётся один раз
    std::map<std::pair<int, int>, int> createdNodes;

    addMiddleNodes(m_elements, VERTICES_IN_ELEMENT, createdNodes);
    addMiddleNodes(m_boundaryElements, VERTICES_IN_BOUNDARY_ELEMENT, createdNodes);
}

// ------------------------------------------------------------------- вывод

void Mesh::printNode(std::ostream& p_out, const Node& p_node) const
{
    p_out << p_node << std::endl;
}

void Mesh::printElement(std::ostream& p_out, const FiniteElement& p_element) const
{
    p_out << p_element << std::endl;
}
