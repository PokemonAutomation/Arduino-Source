/*  Fixed Limit Vector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *      A non-copyable, non-movable vector whose buffer cannot be resized.
 *  It supports emplace_back() and pop_back(). All elements are unchanged
 *  through either operation unless it's the one being popped by pop_back().
 *
 *  There are no requirements of the elements. They need not be copyable nor
 *  movable since they will stay in place.
 *
 */

#ifndef PokemonAutomation_FixedLimitVector_H
#define PokemonAutomation_FixedLimitVector_H

#include <new>
#include <utility>

namespace PokemonAutomation{


template <typename Object>
class FixedLimitVector{
public:
    ~FixedLimitVector();
    FixedLimitVector(const FixedLimitVector&) = delete;
    void operator=(const FixedLimitVector&) = delete;
    FixedLimitVector(FixedLimitVector&& x);
    void operator=(FixedLimitVector&& x);

public:
    FixedLimitVector();
    FixedLimitVector(size_t capacity);

    void reset();
    void reset(size_t capacity);

public:
    size_t size() const{ return m_size; }
    size_t capacity() const{ return m_capacity; }

          Object& operator[](size_t index)      { return m_data[index]; }
    const Object& operator[](size_t index) const{ return m_data[index]; }
          Object& back()      { return m_data[m_size - 1]; }
    const Object& back() const{ return m_data[m_size - 1]; }

    template <class... Args>
    bool emplace_back(Args&&... args);
    void pop_back();

    Object* begin();
    const Object* begin() const;
    Object* end();
    const Object* end() const;

private:
    Object* m_data;
    size_t m_size;
    size_t m_capacity;
};





//  Implementations

template <typename Object>
FixedLimitVector<Object>::~FixedLimitVector(){
    while (m_size > 0){
        pop_back();
    }
    delete[] reinterpret_cast<char*>(m_data);
}
template <typename Object>
FixedLimitVector<Object>::FixedLimitVector(FixedLimitVector&& x)
    : m_data(x.m_data)
    , m_size(x.m_size)
    , m_capacity(x.m_capacity)
{
    x.m_data = nullptr;
    x.m_size = 0;
    x.m_capacity = 0;
}
template <typename Object>
void FixedLimitVector<Object>::operator=(FixedLimitVector&& x){
    reset();
    m_data = x.m_data;
    m_size = x.m_size;
    m_capacity = x.m_capacity;
    x.m_data = nullptr;
    x.m_size = 0;
    x.m_capacity = 0;
}

template <typename Object>
FixedLimitVector<Object>::FixedLimitVector()
    : m_data(nullptr)
    , m_size(0)
    , m_capacity(0)
{}
template <typename Object>
FixedLimitVector<Object>::FixedLimitVector(size_t capacity)
    : m_size(0)
    , m_capacity(capacity)
{
    m_data = reinterpret_cast<Object*>(new char[capacity * sizeof(Object)]);
}

template <typename Object>
void FixedLimitVector<Object>::reset(){
    while (m_size > 0){
        pop_back();
    }
    delete[] reinterpret_cast<char*>(m_data);
    m_data = nullptr;
    m_capacity = 0;
}
template <typename Object>
void FixedLimitVector<Object>::reset(size_t capacity){
    Object* data = reinterpret_cast<Object*>(new char[capacity * sizeof(Object)]);
    while (m_size > 0){
        pop_back();
    }
    delete[] reinterpret_cast<char*>(m_data);
    m_data = data;
    m_capacity = capacity;
}

template <typename Object>
template <class... Args>
bool FixedLimitVector<Object>::emplace_back(Args&&... args){
    if (m_size < m_capacity){
        new (m_data + m_size) Object(std::forward<Args>(args)...);
        m_size++;
        return true;
    }else{
        return false;
    }
}
template <typename Object>
void FixedLimitVector<Object>::pop_back(){
    m_data[--m_size].~Object();
}

template <typename Object>
Object* FixedLimitVector<Object>::begin(){
    return m_data;
}
template <typename Object>
const Object* FixedLimitVector<Object>::begin() const{
    return m_data;
}
template <typename Object>
Object* FixedLimitVector<Object>::end(){
    return m_data + m_size;
}
template <typename Object>
const Object* FixedLimitVector<Object>::end() const{
    return m_data + m_size;
}



}
#endif
