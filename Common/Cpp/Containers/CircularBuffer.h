/*  Circular Buffer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_CircularBuffer_H
#define PokemonAutomation_CircularBuffer_H

#include <new>
#include <utility>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Containers/AlignedMalloc.h"

namespace PokemonAutomation{



template <typename Object>
class CircularBuffer{
public:
    ~CircularBuffer();
    CircularBuffer(CircularBuffer&& x) noexcept;
    void operator=(CircularBuffer&& x) noexcept;
    CircularBuffer(const CircularBuffer& x);
    void operator=(const CircularBuffer& x);

public:
    CircularBuffer();
    CircularBuffer(size_t capacity);

    void clear() noexcept;

    size_t empty() const{ return m_front == m_back; }
    size_t full() const{ return m_back - m_front >= m_capacity; }
    size_t size() const{ return m_back - m_front; }

    const Object& front() const;
          Object& front();
    const Object& operator[](size_t index) const;
          Object& operator[](size_t index);


    template <class... Args>
    Object& push_back(Args&&... args);
    template <class... Args>
    Object* try_push_back(Args&&... args);
    void pop_front();

private:
    size_t m_capacity;
    size_t m_front;
    size_t m_back;
    Object* m_ptr;
};



template <typename Object>
CircularBuffer<Object>::~CircularBuffer(){
    clear();
    aligned_free(m_ptr);
}
template <typename Object>
CircularBuffer<Object>::CircularBuffer(CircularBuffer&& x) noexcept
    : m_capacity(x.m_capacity)
    , m_front(x.m_front)
    , m_back(x.m_back)
    , m_ptr(x.m_ptr)
{
    x.m_capacity = 0;
    x.m_front = 0;
    x.m_back = 0;
    x.m_ptr = nullptr;
}
template <typename Object>
void CircularBuffer<Object>::operator=(CircularBuffer&& x) noexcept{
    if (this == &x){
        return;
    }
    clear();
    aligned_free(m_ptr);
    m_capacity = x.m_capacity;
    m_front = x.m_front;
    m_back = x.m_back;
    m_ptr = x.m_ptr;
    x.m_capacity = 0;
    x.m_front = 0;
    x.m_back = 0;
    x.m_ptr = nullptr;
}
template <typename Object>
CircularBuffer<Object>::CircularBuffer(const CircularBuffer& x)
    : CircularBuffer(x.m_capacity)
{
    size_t size = x.size();
    for (size_t c = 0; c < size; c++){
        push_back(x[c]);
    }
}
template <typename Object>
void CircularBuffer<Object>::operator=(const CircularBuffer& x){
    if (this == &x){
        return;
    }
    CircularBuffer tmp(x);
    *this = std::move(tmp);
}

template <typename Object>
CircularBuffer<Object>::CircularBuffer()
    : m_capacity(0)
    , m_front(0)
    , m_back(0)
    , m_ptr(nullptr)
{}
template <typename Object>
CircularBuffer<Object>::CircularBuffer(size_t capacity)
    : m_capacity(capacity)
    , m_front(0)
    , m_back(0)
    , m_ptr(nullptr)
{
    if (capacity == 0){
        return;
    }
    m_ptr = (Object*)aligned_malloc(capacity * sizeof(Object), alignof(Object));
    if (m_ptr == nullptr){
        throw std::bad_alloc();
    }
}

template <typename Object>
void CircularBuffer<Object>::clear() noexcept{
    while (m_back > m_front){
        pop_front();
    }
}


template <typename Object>
const Object& CircularBuffer<Object>::front() const{
    return m_ptr[m_front];
}
template <typename Object>
Object& CircularBuffer<Object>::front(){
    return m_ptr[m_front];
}
template <typename Object>
const Object& CircularBuffer<Object>::operator[](size_t index) const{
    index += m_front;
    if (index >= m_capacity){
        index -= m_capacity;
    }
    return m_ptr[index];
}
template <typename Object>
Object& CircularBuffer<Object>::operator[](size_t index){
    index += m_front;
    if (index >= m_capacity){
        index -= m_capacity;
    }
    return m_ptr[index];
}

template <typename Object>
template <class... Args>
Object& CircularBuffer<Object>::push_back(Args&&... args){
    if (m_back - m_front >= m_capacity){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "CircularBuffer: Attempted to push while full.");
    }
    size_t back = m_back;
    if (back >= m_capacity){
        back -= m_capacity;
    }
    new (m_ptr + back) Object(std::forward<Args>(args)...);
    m_back++;
    return m_ptr[back];
}
template <typename Object>
template <class... Args>
Object* CircularBuffer<Object>::try_push_back(Args&&... args){
    if (m_back - m_front >= m_capacity){
        return nullptr;
    }
    size_t back = m_back;
    if (back >= m_capacity){
        back -= m_capacity;
    }
    new (m_ptr + back) Object(std::forward<Args>(args)...);
    m_back++;
    return &m_ptr[back];
}

template <typename Object>
void CircularBuffer<Object>::pop_front(){
    if (m_front == m_back){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "CircularBuffer: Attempted to pop while empty.");
    }
    size_t front = m_front;
    m_ptr[front].~Object();
    front++;
    size_t diff = front >= m_capacity ? m_capacity : 0;
    m_front = front - diff;
    m_back -= diff;
}







}
#endif
