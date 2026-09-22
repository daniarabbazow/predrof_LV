# Лабораторная работа № 1. Конечно-элементный загрузчик сетки

Курс «Основы программирования на C++», 1 курс, 2 семестр.

Программа загружает конечно-элементную сетку из файла формата `.aneu`
и выполняет над ней операции поиска и измельчения.

## Структура проекта

| Файл | Назначение |
| --- | --- |
| `src/MeshTypes.h`, `src/MeshTypes.cpp` | вспомогательные типы `Node` и `FiniteElement`, вывод в поток |
| `src/MeshExceptions.h` | иерархия исключений загрузчика |
| `src/Mesh.h`, `src/Mesh.cpp` | класс сетки `Mesh` и предикат `HasThreeNodes` |
| `src/MeshLoader.h` | базовый класс загрузчика с чисто виртуальным методом `loadMesh` |
| `src/AneuMeshLoader.h`, `src/AneuMeshLoader.cpp` | загрузчик формата `.aneu` |
| `src/main.cpp` | консольная программа, имя файла передаётся в аргументах |
| `meshes/MeshExample.aneu` | пример сетки (85 узлов, 150 тетраэдров, 166 треугольников) |

## Сборка и запуск

```bash
make
./mesh_loader meshes/MeshExample.aneu
```

Либо одной командой: `make run`. Очистка: `make clean`.

## Типы данных

`Node` хранит ID узла (>= 1), декартовы координаты `x`, `y`, `z` и флаг `isVertex`
(`true` — узел является вершиной КЭ, `false` — внутренний узел, например середина ребра).

`FiniteElement` хранит ID КЭ (>= 1), ID геометрической области `regionId`
(ID объёма для тетраэдра или ID грани для граничного треугольника)
и список ID узлов `nodeIds`.

## Методы класса Mesh

| Метод | Что делает | Сложность |
| --- | --- | --- |
| `getNodes`, `getElements`, `getBoundaryElements` | контейнеры узлов, КЭ и граничных КЭ | O(1) |
| `getNode(id)` | узел по ID (узлы лежат подряд, узел `n` — по индексу `n - 1`) | O(1) |
| `findElementsByThreeNodes(id1, id2, id3)` | ID тетраэдров по трём вершинным узлам, через `std::find_if` и предикат `HasThreeNodes` | O(E) |
| `findElementsByEdge(id1, id2)` | ID тетраэдров с общим ребром | O(E) |
| `getBoundaryNodeIds(boundaryId)` | ID граничных узлов заданной границы | O(B log B) |
| `getElementIdsByRegion(regionId)` | ID тетраэдров заданной области | O(E) |
| `getBoundaryElementIds(boundaryId)` | ID граничных КЭ заданной границы | O(B) |
| `getNodeNeighbours()` | контейнер, элемент с индексом `id - 1` которого хранит ID всех соседних по рёбрам узлов узла `id` | O((E + B) log N) |
| `splitEdges()` | вставка новых узлов в середины рёбер всех КЭ | O((E + B) log N) |
| `printNode`, `printElement` | отформатированный вывод в заданный поток | O(1) |

Здесь `N` — число узлов, `E` — число тетраэдров, `B` — число граничных КЭ.
Квадратичных по размеру сетки методов нет.

Поиск и выборка построены на алгоритмах STL (`std::find_if`, `std::find`,
`std::for_each`, `std::transform`) и контейнерах `std::vector`, `std::set`, `std::map`.

### Про splitEdges

Метод добавляет узел в середину каждого ребра КЭ. Одно ребро принадлежит сразу
нескольким элементам, поэтому используется словарь `std::map`, ключ которого —
пара ID узлов ребра (меньший ID первым). Благодаря этому на общем ребре узел
создаётся ровно один раз, а в списки узлов всех КЭ, которым принадлежит ребро,
попадает один и тот же ID.

Словарь общий для объёмных и граничных КЭ, поэтому узел на ребре граничного
треугольника совпадает с узлом на том же ребре соседнего тетраэдра.

Новые узлы дописываются в конец списка узлов КЭ в порядке перечисления рёбер:
для тетраэдра `(0,1) (0,2) (0,3) (1,2) (1,3) (2,3)`, для треугольника `(0,1) (0,2) (1,2)`.
В результате тетраэдр становится 10-узловым, а граничный треугольник — 6-узловым.
У всех новых узлов флаг `isVertex` равен `false`.

На сетке из `meshes/MeshExample.aneu` получается 317 уникальных рёбер,
то есть число узлов растёт с 85 до 402.

## Обработка ошибок

Все ошибки загрузки сообщаются через исключения:

```
MeshException              базовое исключение (наследник std::runtime_error)
├── FileOpenException      не удалось открыть файл сетки
├── FileFormatException    неверный формат файла (не та размерность, не тот тип КЭ,
│                          файл оборван, ID узла вне диапазона и т.п.)
└── BadIdException         обращение к несуществующему узлу
```

Примеры сообщений:

```
Mesh error: Can not open mesh file: no_such_file.aneu
Mesh error: Wrong mesh file format: only 3D meshes are supported
Mesh error: Wrong mesh file format: block "finite elements", element 2 refers to node 999
Mesh error: Wrong mesh file format: can not read node 50
```

## Добавление нового формата

Чтобы поддержать другой формат файла сетки, достаточно создать класс,
производный от `MeshLoader`, и переопределить в нём метод `loadMesh`.
Класс `Mesh` при этом менять не требуется.

## Пример вывода

```
Mesh is loaded from file meshes/MeshExample.aneu
Nodes:             85
Finite elements:   150
Boundary elements: 166

First node and first finite element:
Node     1: (  0.000000,   0.000000,   0.000000) vertex
FE     1: region =   1, nodes = 13 14 10 79

Finite elements with nodes 13, 14, 10: 1
Finite elements with edge (13, 14): 1
Finite elements of region 1: 150 items
Boundary elements of boundary 12: 8 items
Nodes of boundary 12: 1 2 3 4 61 62 63 64 65 66

Neighbours of node 1: 2 36 66

Inserting nodes into the middles of edges...
Nodes now:                    402
Nodes in a finite element:    10
Nodes in a boundary element:  6

First finite element after splitting:
FE     1: region =   1, nodes = 13 14 10 79 86 87 88 89 90 91
Last node after splitting:
Node   402: (  0.563781,   0.000000,   0.000000) inner
```

## Формат файла .aneu

```
<Количество узлов> <Размерность пространства>
X1 X2 X3
...
<Количество КЭ> <Количество узлов в одном КЭ>
<ID материала> N1 N2 N3 N4
...
<Количество поверхностных КЭ> <Количество узлов в одном поверхностном КЭ>
<ID поверхности> N1 N2 N3
...
```

ID узла — число от 1 до количества узлов. ID узлов и КЭ в файле явно не записаны,
они определяются порядком строк, начиная с 1.
