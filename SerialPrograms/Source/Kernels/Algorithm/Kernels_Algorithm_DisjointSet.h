/*  Disjoint Set
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_Algorithm_DisjointSet_H
#define PokemonAutomation_Kernels_Algorithm_DisjointSet_H

#include <cstddef>
#include <vector>

namespace PokemonAutomation{
namespace Kernels{

// https://en.wikipedia.org/wiki/Disjoint-set_data_structure
class DisjointSet{
public:
    // size: num elements
    DisjointSet(size_t size);

    // Find the representative element of the set x belongs to
    // x range: [0, size)
    size_t find(size_t x);

    // Merge the two sets x and y belong to
    // x, y range: [0, size)
    void merge(size_t x, size_t y);

    // Size of the set where element x belongs to.
    // x range: [0, size)
    size_t set_size(size_t x);


private:
    std::vector<size_t> m_parent;
    std::vector<size_t> m_size;
};


}
}
#endif
