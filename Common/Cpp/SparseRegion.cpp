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

#include <vector>
#include <sstream>
#include "Common/Cpp/Exceptions.h"
#include "SparseRegion.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{



template <typename Type>
Type parse_ui(const char* str){
    Type x = 0;
    char ch = *str++;
    while (ch >= '0' && ch <= '9'){
        x *= 10;
        x += ch - '0';
        ch = *str++;
    }
    return x;
}



template <typename Type>
Region<Type>::Region(Type is, Type ie)
    : s(is), e(ie)
{
    if (e <= s){
//        cout << "s = " << s << ", e = " << e << endl;
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "s < e");
    }
}
template <typename Type>
Region<Type> Region<Type>::parse_str(const std::string& str, bool inclusive_end){
    const char* ptr = str.c_str();

    //  Skip to the first digit.
    while (true){
        char ch = *ptr;
        if (ch == '\0'){
            throw ParseException("Unexpected end of string.");
        }
        if ('0' <= ch && ch <= '9'){
            break;
        }
        ptr++;
    }

    //  Parse the range.
    std::string substr = ptr;

    Type s = parse_ui<Type>(substr.c_str());
    Type e = s;
    Type pos = substr.find('-');
    if (pos != std::string::npos){
        e = parse_ui<Type>(&substr[pos + 1]);
    }

    if (inclusive_end){
        e++;
    }

    return Region(s, e);
}
template <typename Type>
bool Region<Type>::operator==(const Region& x) const{
    return s == x.s && e == x.e;
}




template <typename Type>
SparseRegion<Type>::SparseRegion(const std::string& str, bool inclusive_end){
    std::istringstream stream(str);
    std::string token;
    std::vector<std::string> tokens;

    while (std::getline(stream, token, ' ')) {
        *this |= Region<Type>::parse_str(token, inclusive_end);
    }
}

template <typename Type>
bool SparseRegion<Type>::operator==(const Region<Type>& region) const{
    if (m_ranges.size() != 1){
        return false;
    }
    auto iter = m_ranges.begin();
    return Region<Type>(iter->first, iter->second) == region;
}


template <typename Type>
void SparseRegion<Type>::operator|=(const Region<Type>& region){
    using Iterator = typename std::map<Type, Type>::iterator;

    Type s = region.s;
    Type e = region.e;

    auto ret = m_ranges.emplace(s, e);
    Iterator node = ret.first;

    //  Node already exists. Merge with it.
    if (!ret.second){
        e = std::max(e, node->second);
        node->second = e;
    }

    //  If previous node exists, attempt to merge with it.
    if (node != m_ranges.begin()){
        Iterator prev = node;
        --prev;
        if (prev->second >= s){
            //  Merge with previous node.
            s = prev->first;
            e = std::max(e, prev->second);
            prev->second = e;
            m_ranges.erase(node);
            node = prev;
        }
    }

    //  Attempt to merge with later nodes.
    while (true){
        Iterator next = node;
        ++next;

        //  No next node.
        if (next == m_ranges.end()){
            return;
        }

        //  Next node does not touch current node.
        if (e < next->first){
            return;
        }

        //  Merge with next node.
        e = std::max(e, next->second);
        node->second = e;
        m_ranges.erase(next);
    }
}


template <typename Type>
std::string SparseRegion<Type>::tostr(bool inclusive_end) const{
    std::string ret;
    for (const auto& item : m_ranges){
        if (!ret.empty()){
            ret += ", ";
        }
        ret += std::to_string(item.first);
        if (item.second - item.first != 1){
            ret += "-";
            ret += std::to_string(inclusive_end ? item.second - 1 : item.second);
        }
    }
    return ret;
}
template <typename Type>
std::string SparseRegion<Type>::dump() const{
    std::string ret("Sparse Region:");
    for (const auto& item : m_ranges){
        ret += "    [" + std::to_string(item.first) + ", " + std::to_string(item.second) + ")\n";
    }
    return ret;
}



template struct Region<size_t>;
template class SparseRegion<size_t>;



}
