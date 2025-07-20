/*  Sparse Array
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_SparseArray_H
#define PokemonAutomation_SparseArray_H

#include <string.h>
#include <string>
#include <map>

namespace PokemonAutomation{


struct SparseArrayBlock{
    size_t address;
    std::string data;

    SparseArrayBlock(size_t p_address, const char* str)
        : address(p_address)
        , data(str, str + strlen(str))
    {}
    SparseArrayBlock(size_t p_address, std::initializer_list<char> p_data)
        : address(p_address)
        , data(p_data)
    {}
};


class SparseArray{
public:
    SparseArray() = default;
    SparseArray(std::initializer_list<SparseArrayBlock> list){
        for (auto& item : list){
            write(item.address, item.data.size(), item.data.data());
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
            ret += item.second;
            ret += "\n";
        }
        return ret;
    }
#endif

private:
    std::map<size_t, std::string> m_data;
};




}
#endif
