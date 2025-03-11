/*  Stream Converters
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <string.h>
#include "Exceptions.h"
#include "Containers/AlignedVector.tpp"
#include "StreamConverters.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{


StreamConverter::StreamConverter(
    size_t object_size_in,
    size_t object_size_out,
    size_t buffer_capacity
)
    : m_object_size_in(object_size_in)
    , m_object_size_out(object_size_out)
    , m_buffer_capacity(buffer_capacity)
    , m_buffer(object_size_out * buffer_capacity)
{}
StreamConverter::~StreamConverter(){}
void StreamConverter::add_listener(StreamListener& listener){
    if (listener.object_size != m_object_size_out){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Mismatching object size.");
    }
    m_listeners.insert(&listener);
}
void StreamConverter::remove_listener(StreamListener& listener){
    m_listeners.erase(&listener);
}
void StreamConverter::push_objects(const void* data, size_t objects){
    while (objects > 0){
        size_t block = std::min(objects, m_buffer_capacity);
        convert(m_buffer.data(), data, block);
        data = (char*)data + block * m_object_size_in;
        objects -= block;
        for (StreamListener* listener : m_listeners){
            listener->on_objects(m_buffer.data(), block);
        }
    }
}


//void print_u8(const uint8_t* ptr, size_t len);


MisalignedStreamConverter::MisalignedStreamConverter(
    size_t object_size_in,
    size_t object_size_out,
    size_t buffer_capacity
)
    : m_object_size_in(object_size_in)
    , m_object_size_out(object_size_out)
    , m_edge(object_size_in)
    , m_buffer_capacity(buffer_capacity)
    , m_buffer(object_size_out * buffer_capacity)
{}
MisalignedStreamConverter::~MisalignedStreamConverter(){}
void MisalignedStreamConverter::add_listener(StreamListener& listener){
    if (listener.object_size != m_object_size_out){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Mismatching object size.");
    }
    m_listeners.insert(&listener);
}
void MisalignedStreamConverter::remove_listener(StreamListener& listener){
    m_listeners.erase(&listener);
}
void MisalignedStreamConverter::push_bytes(const void* data, size_t bytes){
//    cout << "push: ";
//    print_u8((uint8_t*)data, bytes);

    //  Fill the edge block.
    if (m_edge_size > 0){
//        cout << "m_edge_size = " << m_edge_size << endl;
        size_t block = std::min(m_object_size_in - m_edge_size, bytes);
        memcpy(m_edge.data() + m_edge_size, data, block);
        m_edge_size += block;
        data = (char*)data + block;
        bytes -= block;
    }

    size_t stored = 0;

    //  Process completed edge block.
    if (m_edge_size >= m_object_size_in){
//        print_u8((uint8_t*)m_edge.data(), m_edge_size);
        convert(m_buffer.data() + stored * m_object_size_out, m_edge.data(), 1);
        m_edge_size = 0;
        stored++;
    }

    size_t objects = bytes / m_object_size_in;
    while (stored + objects > 0){
        size_t block = std::min(objects, m_buffer_capacity - stored);
//        cout << "stored = " << stored << endl;
//        cout << "block = " << block << endl;
//        print_u8((uint8_t*)data, block * m_object_size_in);
        convert(m_buffer.data() + stored * m_object_size_out, data, block);
        data = (char*)data + block * m_object_size_in;
        bytes -= block * m_object_size_in;
        stored += block;
        objects -= block;
        for (StreamListener* listener : m_listeners){
            listener->on_objects(m_buffer.data(), stored);
        }
        stored = 0;
    }

//    cout << "left = " << bytes << endl;

    //  Copy remaining bytes into edge buffer.
    memcpy(m_edge.data(), data, bytes);
    m_edge_size = bytes;
}






}
