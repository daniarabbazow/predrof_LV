#include "MeshTypes.h"

#include <iomanip>

std::ostream& operator<<(std::ostream& p_out, const Node& p_node)
{
    p_out << "Node " << std::setw(5) << p_node.id << ": ("
          << std::fixed << std::setprecision(6)
          << std::setw(10) << p_node.x << ", "
          << std::setw(10) << p_node.y << ", "
          << std::setw(10) << p_node.z << ") "
          << (p_node.isVertex ? "vertex" : "inner");

    return p_out;
}

std::ostream& operator<<(std::ostream& p_out, const FiniteElement& p_element)
{
    p_out << "FE " << std::setw(5) << p_element.id
          << ": region = " << std::setw(3) << p_element.regionId
          << ", nodes =";

    for (size_t i = 0; i < p_element.nodeIds.size(); ++i)
        p_out << " " << p_element.nodeIds[i];

    return p_out;
}
