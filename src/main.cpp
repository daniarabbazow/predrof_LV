#include "MeshLoader.h"
#include "MeshExceptions.h"

#include <iostream>
#include <vector>

// Вывод контейнера ID в одну строку
void printIds(const std::vector<int>& p_ids)
{
    if (p_ids.empty())
    {
        std::cout << " (empty)";
        return;
    }

    for (size_t i = 0; i < p_ids.size(); ++i)
        std::cout << " " << p_ids[i];
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <mesh file .aneu>" << std::endl;
        return 1;
    }

    try
    {
        AneuMeshLoader loader;
        Mesh mesh = loader.loadMesh(argv[1]);

        std::cout << "Mesh is loaded from file " << argv[1] << std::endl;
        std::cout << "Nodes:             " << mesh.getNodes().size() << std::endl;
        std::cout << "Finite elements:   " << mesh.getElements().size() << std::endl;
        std::cout << "Boundary elements: " << mesh.getBoundaryElements().size()
                  << std::endl << std::endl;

        // Пример вывода узла и КЭ
        std::cout << "First node and first finite element:" << std::endl;
        mesh.printNode(std::cout, mesh.getNodes().front());
        mesh.printElement(std::cout, mesh.getElements().front());
        std::cout << std::endl;

        // Поиск КЭ по трём узлам (берём первые три узла первого КЭ)
        const FiniteElement& first = mesh.getElements().front();
        int n1 = first.nodeIds[0];
        int n2 = first.nodeIds[1];
        int n3 = first.nodeIds[2];

        std::cout << "Finite elements with nodes " << n1 << ", " << n2 << ", " << n3 << ":";
        printIds(mesh.findElementsByThreeNodes(n1, n2, n3));
        std::cout << std::endl;

        // Поиск КЭ с общим ребром
        std::cout << "Finite elements with edge (" << n1 << ", " << n2 << "):";
        printIds(mesh.findElementsByEdge(n1, n2));
        std::cout << std::endl;

        // КЭ заданной области и граничные КЭ заданной границы
        int regionId = first.regionId;
        int boundaryId = mesh.getBoundaryElements().front().regionId;

        std::cout << "Finite elements of region " << regionId << ": "
                  << mesh.getElementIdsByRegion(regionId).size() << " items" << std::endl;
        std::cout << "Boundary elements of boundary " << boundaryId << ": "
                  << mesh.getBoundaryElementIds(boundaryId).size() << " items" << std::endl;

        std::cout << "Nodes of boundary " << boundaryId << ":";
        printIds(mesh.getBoundaryNodeIds(boundaryId));
        std::cout << std::endl << std::endl;

        // Соседние по рёбрам узлы
        std::vector<std::vector<int> > neighbours = mesh.getNodeNeighbours();
        std::cout << "Neighbours of node 1:";
        printIds(neighbours[0]);
        std::cout << std::endl << std::endl;

        // Вставка узлов в середины рёбер
        std::cout << "Inserting nodes into the middles of edges..." << std::endl;
        mesh.splitEdges();

        std::cout << "Nodes now:                    " << mesh.getNodes().size() << std::endl;
        std::cout << "Nodes in a finite element:    "
                  << mesh.getElements().front().nodeIds.size() << std::endl;
        std::cout << "Nodes in a boundary element:  "
                  << mesh.getBoundaryElements().front().nodeIds.size()
                  << std::endl << std::endl;

        std::cout << "First finite element after splitting:" << std::endl;
        mesh.printElement(std::cout, mesh.getElements().front());
        std::cout << "Last node after splitting:" << std::endl;
        mesh.printNode(std::cout, mesh.getNodes().back());
    }
    catch (const MeshException& e)
    {
        std::cerr << "Mesh error: " << e.what() << std::endl;
        return 1;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
