/*  Sparse Array
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_SparseArray_H
#define PokemonAutomation_SparseArray_H

#include <string>
#include <vector>
#include <map>

namespace PokemonAutomation{


struct SparseArrayBlock{
    size_t address;
    size_t bytes;
    const void* data;
};


class SparseArray{
public:
    SparseArray() = default;
    SparseArray(std::initializer_list<SparseArrayBlock> list){
        for (auto& item : list){
            write(item.address, item.bytes, item.data);
        }
    }

    void write(size_t address, size_t bytes, const void* data);
    void read(size_t address, size_t bytes, void* data) const;

#if 1
    std::string dump() const{
        std::string ret;
        for (const auto& item : m_data){
            ret += "{";
            ret += std::to_string(item.first);
            ret += " : ";
            ret += std::to_string(item.first + item.second.size());
            ret += "} = ";
            ret += std::string((const char*)item.second.data(), item.second.size());
            ret += "\n";
        }
        return ret;
    }
#endif

private:
    std::map<size_t, std::vector<uint8_t>> m_data;
};




}
#endif
