/*  Sparse Array
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <string.h>
#include "SparseArray.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{



void SparseArray::write(size_t address, size_t bytes, const void* data){
    if (m_data.empty()){
        m_data.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(address),
            std::forward_as_tuple((char*)data, (char*)data + bytes)
        );
        return;
    }

    //  Below bottom.
    size_t top = address + bytes;
    size_t lowest = m_data.begin()->first;
    if (top < lowest){
        m_data.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(address),
            std::forward_as_tuple((char*)data, (char*)data + bytes)
        );
        return;
    }

    //  Above top.
    size_t highest = m_data.rbegin()->first + m_data.rbegin()->second.size();
    if (highest < address){
        m_data.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(address),
            std::forward_as_tuple((char*)data, (char*)data + bytes)
        );
        return;
    }

    //  Get the first and last blocks that will be affected.
    auto lower = m_data.upper_bound(address);
    if (lower != m_data.begin()){
        --lower;
    }
    if (lower->first + lower->second.size() < address){
        ++lower;
    }
    auto upper = m_data.lower_bound(top);
    if (upper != m_data.begin()){
        --upper;
    }

    lowest = lower->first;
    highest = upper->first + upper->second.size();
    if (lower == upper && lowest <= address && top <= highest){
        //  Completely inside.
        memcpy(lower->second.data() + address - lowest, data, bytes);
        return;
    }

    lowest = std::min(lowest, address);
    highest = std::max(highest, top);

    std::string block(highest - lowest, 0);

    //  Copy in the partial blocks.
    bool one_block = lower == upper;
    {
        size_t shift = lower->first - lowest;
        memcpy(
            block.data() + shift,
            lower->second.data(),
            lower->second.size()
        );
    }
    if (!one_block){
        size_t shift = highest - upper->first - upper->second.size();
        memcpy(
            block.data() + block.size() - upper->second.size() - shift,
            upper->second.data(),
            upper->second.size()
        );
    }

//    cout << "lower = " << lower->first << endl;
//    cout << "upper = " << upper->first << endl;
//    cout << "lowest  = " << lowest << endl;
//    cout << "highest = " << highest << endl;

    //  Copy the newly written block.
    memcpy(block.data() + address - lowest, data, bytes);

//    cout << std::string((char*)block.data(), block.size()) << endl;

    //  Insert into table.
    if (lower->first == lowest){
//        cout << "replace" << endl;
        lower->second = std::move(block);
        ++lower;
    }else{
//        cout << "add" << endl;
        auto iter = m_data.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(lowest),
            std::forward_as_tuple(std::move(block))
        ).first;
        m_data.erase(lower);
        ++iter;
        lower = iter;
    }

    //  Remove all the blocks that it touched.
    if (!one_block){
        while (lower != upper){
            lower = m_data.erase(lower);
        }
        m_data.erase(lower);
    }
}
void SparseArray::read(size_t address, size_t bytes, void* data) const{
    if (m_data.empty()){
        return;
    }

    //  Below bottom.
    size_t top = address + bytes;
    size_t lowest = m_data.begin()->first;
    if (top < lowest){
        return;
    }

    //  Above top.
    size_t highest = m_data.rbegin()->first + m_data.rbegin()->second.size();
    if (highest < address){
        return;
    }

    auto lower = m_data.upper_bound(address);
    if (lower != m_data.begin()){
        --lower;
    }

    while (lower != m_data.end() && bytes > 0){
        size_t block = lower->first;
        if (block >= top){
            break;
        }

//        cout << "bytes = " << bytes << endl;
//        cout << "address = " << address << endl;
//        cout << "block = " << block << endl;

        const char* ptr = lower->second.data();
        size_t len = lower->second.size();

        if (block < address){
//            cout << "block is before" << endl;
            size_t shift = address - block;
            block = address;
            ptr += shift;
            len -= shift;   //  Cannot go negative since it's guaranteed to overlap with region.
        }
        if (block > address){
//            cout << "block is later" << endl;
            size_t shift = block - address;
//            cout << "shift = " << shift << endl;
            address = block;
            data = (char*)data + shift;
            bytes -= shift; //  Cannot go negative since it's guaranteed to overlap with block.
        }

        len = std::min(len, bytes);

//        cout << "len = " << len << endl;

        memcpy(data, ptr, len);
        data = (char*)data + len;
        address += len;
        bytes -= len;
        ++lower;
    }

}




}
