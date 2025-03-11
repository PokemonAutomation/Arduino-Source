/*  Disjoint Set
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <vector>
#include <numeric>

#include "Kernels_Algorithm_DisjointSet.h"

namespace PokemonAutomation{
namespace Kernels{

DisjointSet::DisjointSet(size_t size) : m_parent(size), m_size(size, 1){
    std::iota(m_parent.begin(), m_parent.end(), 0);
}

size_t DisjointSet::find(size_t x){
    // while x is not root
    while (m_parent[x] != x){
        m_parent[x] = m_parent[m_parent[x]];
        x = m_parent[x];
    }
    return x;
}

void DisjointSet::merge(size_t x, size_t y){
    x = find(x);
    y = find(y);

    if (x == y){
        // input parameter x and y are from the same set:
        return;
    }

    // make sure size(x) >= size(y)
    if (m_size[x] < m_size[y]){
        std::swap(x, y);
    }

    m_parent[y] = x;
    m_size[x] += m_size[y];
}

size_t DisjointSet::set_size(size_t x){
    x = find(x);  // find root
    return m_size[x];
}

}
}
