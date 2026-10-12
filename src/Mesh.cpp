#include "Mesh.h"
#include "MeshExceptions.h"

#include <algorithm>
#include <set>
#include <sstream>

// ---------------------------------------------------------------- предикат

// operator() предиката: подходит ли элемент.
// Как работает: по очереди ищет каждый из трёх номеров в списке узлов
// элемента (std::find). Если нашлись все три - элемент подходит.
bool HasThreeNodes::operator()(const FiniteElement& p_element) const
{
    const std::vector<int>& nodes = p_element.nodeIds;

    // КЭ подходит, если в его списке узлов есть все три заданных ID
    return std::find(nodes.begin(), nodes.end(), m_id1) != nodes.end()
        && std::find(nodes.begin(), nodes.end(), m_id2) != nodes.end()
        && std::find(nodes.begin(), nodes.end(), m_id3) != nodes.end();
}

// ------------------------------------------------------------ конструкторы

// Конструктор пустой сетки.
// Как работает: делать ничего не нужно - все три списка (вектора)
// сами создаются пустыми.
Mesh::Mesh()
{}

// Конструктор сетки из готовых списков.
// Как работает: копирует переданные списки в поля сетки
// через список инициализации.
Mesh::Mesh(const std::vector<Node>& p_nodes,
           const std::vector<FiniteElement>& p_elements,
           const std::vector<FiniteElement>& p_boundaryElements)
    : m_nodes(p_nodes)
    , m_elements(p_elements)
    , m_boundaryElements(p_boundaryElements)
{}

// ------------------------------------------------------- доступ к данным

// getNodes: все узлы сетки.
// Как работает: возвращает ссылку на поле m_nodes, без копирования.
const std::vector<Node>& Mesh::getNodes() const
{
    return m_nodes;
}

// getElements: все тетраэдры.
// Как работает: возвращает ссылку на поле m_elements, без копирования.
const std::vector<FiniteElement>& Mesh::getElements() const
{
    return m_elements;
}

// getBoundaryElements: все граничные треугольники.
// Как работает: возвращает ссылку на поле m_boundaryElements, без копирования.
const std::vector<FiniteElement>& Mesh::getBoundaryElements() const
{
    return m_boundaryElements;
}

// getNode: узел по номеру.
// Как работает: сначала проверяет, что номер есть в сетке (от 1 до числа
// узлов), иначе бросает исключение. Потом сразу берёт узел из вектора
// по индексу - искать его не нужно.
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

// findElementsByThreeNodes: тетраэдры, в которых есть три заданных узла.
// Как работает: создаёт предикат HasThreeNodes с этими номерами и в цикле
// вызывает std::find_if. Каждый вызов находит следующий подходящий тетраэдр,
// начиная с места, где остановился прошлый. Номер найденного добавляется
// в результат. Когда find_if дошёл до конца списка - всё найдено.
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

// findElementsByEdge: тетраэдры с общим ребром.
// Как работает: проходит по всем тетраэдрам (std::for_each) и берёт те,
// в которых есть оба узла ребра. В тетраэдре любые две вершины соединены
// ребром, поэтому такой проверки достаточно.
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

// getElementIdsByRegion: тетраэдры заданной области.
// Как работает: проходит по всем тетраэдрам и собирает номера тех,
// у которых номер области совпадает с заданным.
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

// getBoundaryElementIds: граничные треугольники заданной границы.
// Как работает: так же, как getElementIdsByRegion, только проходит
// по списку граничных элементов.
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

// getBoundaryNodeIds: узлы на заданной границе.
// Как работает: проходит по треугольникам этой границы и складывает все их
// узлы в множество std::set - оно само убирает повторы и сортирует номера.
// В конце множество переписывается в вектор.
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

// makeEdges: список рёбер элемента.
// Как работает: двумя вложенными циклами перебирает все пары вершин (i, j),
// где i < j, - каждая такая пара и есть ребро. Условие i < j нужно, чтобы
// не взять одно ребро дважды, как (0,1) и (1,0), и не взять пару (0,0).
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

// getNodeNeighbours: соседи каждого узла.
// Как работает: для каждого узла заводится пустое множество соседей.
// Потом перебираются все элементы и все их рёбра: для ребра (a, b) узел b
// записывается в соседи a, а a - в соседи b. Одно ребро встречается в
// нескольких элементах, но множества сами убирают повторы.
// В конце множества переводятся в векторы.
// Учитываются рёбра между вершинами элементов, поэтому метод рассчитан на сетку
// до splitEdges: у узлов в серединах рёбер список соседей останется пустым.
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

// addMiddleNodes: вставка узлов в середины рёбер для одного списка элементов.
// Как работает: для каждого элемента и каждого его ребра:
//   1) делает ключ ребра - пару номеров его узлов, меньший номер первым;
//   2) ищет этот ключ в словаре p_createdNodes;
//   3) если не нашёл - узла на этом ребре ещё нет: создаёт узел посередине
//      (каждая координата - полусумма координат концов ребра) и запоминает
//      его в словаре;
//   4) номер узла (нового или созданного раньше) дописывает в конец
//      списка узлов элемента.
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

// splitEdges: вставка узлов в середины рёбер всей сетки.
// Как работает: заводит один общий словарь "ребро -> номер узла" и вызывает
// addMiddleNodes сначала для тетраэдров, потом для треугольников.
// Словарь общий, поэтому треугольник на поверхности получает тот же узел,
// что и тетраэдр, которому принадлежит это ребро.
void Mesh::splitEdges()
{
    // Словарь "ребро -> ID созданного на нём узла" общий для объёмных
    // и граничных КЭ, поэтому на общем ребре узел создаётся один раз
    std::map<std::pair<int, int>, int> createdNodes;

    addMiddleNodes(m_elements, VERTICES_IN_ELEMENT, createdNodes);
    addMiddleNodes(m_boundaryElements, VERTICES_IN_BOUNDARY_ELEMENT, createdNodes);
}

// ------------------------------------------------------------------- вывод

// printNode: вывод узла в заданный поток.
// Как работает: печатает узел через operator<< из MeshTypes.h
// и добавляет перевод строки.
void Mesh::printNode(std::ostream& p_out, const Node& p_node) const
{
    p_out << p_node << std::endl;
}

// printElement: вывод элемента в заданный поток.
// Как работает: печатает элемент через operator<< из MeshTypes.h
// и добавляет перевод строки.
void Mesh::printElement(std::ostream& p_out, const FiniteElement& p_element) const
{
    p_out << p_element << std::endl;
}
