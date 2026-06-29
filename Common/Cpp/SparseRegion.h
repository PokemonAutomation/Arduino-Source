/*  Sparse Region
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  This file is taken from:
 *  https://github.com/Mysticial/y-cruncher/blob/master/trunk/Source/PublicLibs/BasicLibs/SparseRegion.h
 *
 *  File taken with permission from Mysticial (founder of both projects).
 *
 */

#ifndef PokemonAutomation_SparseRegion_H
#define PokemonAutomation_SparseRegion_H

#include <string>
#include <map>

namespace PokemonAutomation{


template <typename Type>
struct Region{
    const Type s;
    const Type e;

    Region(Type is, Type ie);
    static Region parse_str(const std::string& str, bool inclusive_end);
    bool operator==(const Region& x) const;
};


template <typename Type>
class SparseRegion{
public:
    SparseRegion() = default;
    SparseRegion(const Region<Type>& region){
        m_ranges[region.s] = region.e;
    }
    SparseRegion(const std::string& str, bool inclusive_end);

    bool empty() const{
        return m_ranges.empty();
    }

    bool operator==(const Region<Type>& region) const;
    void operator|=(const Region<Type>& region);

    std::string tostr(bool inclusive_end) const;
    std::string dump() const;

public:
    auto cbegin() const{ return m_ranges.cbegin(); }
    auto begin () const{ return m_ranges.cbegin(); }
    auto begin ()      { return m_ranges.begin(); }
    auto cend  () const{ return m_ranges.cend(); }
    auto end   () const{ return m_ranges.cend(); }
    auto end   ()      { return m_ranges.end(); }

private:
    std::map<Type, Type> m_ranges;
};





}
#endif
