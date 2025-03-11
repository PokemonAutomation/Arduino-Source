/*  Box Set
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *      This class is experimental. It currently doesn't actually accomplish
 *  what it was intended for.
 *
 */

#ifndef PokemonAutomation_BoxSet_H
#define PokemonAutomation_BoxSet_H

#include <string>
#include <map>
#include "Common/Cpp/Rectangle.h"
#include "Common/Cpp/Exceptions.h"

namespace PokemonAutomation{



//
//  An unordered container holding a set of boxes.
//
//  Iterating this container normally will iterate all the boxes in a fixed, but
//  undefined order.
//
//  This class provides "axis iterators" that allow you to iterate the elements
//  in order of each of the axis points on the box. This lets you efficiently
//  iterate boxes that overlap with a point or another box.
//
template <typename Type>
class BoxSet{
public:
    size_t size() const{ return m_boxes.size(); }
    std::string dump() const;

public:
    class Entry;

    //  Normal iterators. These iterate all the elements in a fixed, but
    //  undefined order.
    class iterator;
    class const_iterator;

    const_iterator cbegin() const;
    const_iterator begin () const;
          iterator begin ();

    const_iterator cend() const;
    const_iterator end () const;
          iterator end ();

public:
    //  Axis iterator. These iterate the elements in increasing order either on
    //  the X or Y axis.

    using axis_iterator = typename std::multimap<Type, Entry*>::const_iterator;

    axis_iterator end_min_x() const{ return m_min_x.end(); }
    axis_iterator end_max_x() const{ return m_max_x.end(); }
    axis_iterator end_min_y() const{ return m_min_y.end(); }
    axis_iterator end_max_y() const{ return m_max_y.end(); }

    axis_iterator lower_bound_min_x(Type x) const{ return m_min_x.lower_bound(x); }
    axis_iterator upper_bound_min_x(Type x) const{ return m_min_x.upper_bound(x); }
    axis_iterator lower_bound_max_x(Type x) const{ return m_max_x.lower_bound(x); }
    axis_iterator upper_bound_max_x(Type x) const{ return m_max_x.upper_bound(x); }
    axis_iterator lower_bound_min_y(Type y) const{ return m_min_y.lower_bound(y); }
    axis_iterator upper_bound_min_y(Type y) const{ return m_min_y.upper_bound(y); }
    axis_iterator lower_bound_max_y(Type y) const{ return m_max_y.lower_bound(y); }
    axis_iterator upper_bound_max_y(Type y) const{ return m_max_y.upper_bound(y); }

public:
    //  Insert box into set and return a normal iterator to it.
    iterator insert(const Rectangle<Type>& box);

    //  Erase an iterator and return the next iterator. This will invalidate
    //  all other iterators pointing to the same element.
    iterator erase(iterator item);
    axis_iterator erase(axis_iterator item);

public:
    //  Advanced Functions

    //  Iterate on all boxes which either contain this point or on its border.
    template <typename Func>
    void iterate_in_or_on(Type x, Type y, Func&& lambda);


private:
    static size_t get_key(const Rectangle<Type>& box);

private:
    std::multimap<Type, Entry> m_boxes;
    std::multimap<Type, Entry*> m_min_x;
    std::multimap<Type, Entry*> m_max_x;
    std::multimap<Type, Entry*> m_min_y;
    std::multimap<Type, Entry*> m_max_y;
};



template <typename Type>
class BoxSet<Type>::Entry{
public:
    Entry(const Rectangle<Type>& box)
        : m_box(box)
    {}

    operator const Rectangle<Type>&() const{ return m_box; }
    const Rectangle<Type>* operator->() const{ return &m_box; }
    const Rectangle<Type>& box() const{ return m_box; }

private:
    friend class BoxSet<Type>;
    Rectangle<Type> m_box;
    typename std::multimap<Type, Entry*>::iterator m_min_x;
    typename std::multimap<Type, Entry*>::iterator m_max_x;
    typename std::multimap<Type, Entry*>::iterator m_min_y;
    typename std::multimap<Type, Entry*>::iterator m_max_y;
};


template <typename Type>
class BoxSet<Type>::iterator{
public:
    bool operator==(const iterator& iter) const{ return m_iter == iter.m_iter; }

    iterator operator++(){
        ++m_iter;
        return *this;
    }
    auto operator*() const{ return *m_iter; }
    auto operator*(){ return *m_iter; }
    auto operator->() const{ return m_iter.operator->(); }
    auto operator->(){ return m_iter.operator->(); }

    axis_iterator iterator_min_x() const{
        return m_iter->second.m_min_x;
    }
    axis_iterator iterator_max_x() const{
        return m_iter->second.m_max_x;
    }
    axis_iterator iterator_min_y() const{
        return m_iter->second.m_min_y;
    }
    axis_iterator iterator_max_y() const{
        return m_iter->second.m_max_y;
    }

private:
    friend class BoxSet<Type>;
    friend class const_iterator;

    typename std::multimap<Type, Entry>::iterator m_iter;

    iterator(typename std::multimap<Type, Entry>::iterator iter)
        : m_iter(iter)
    {}
};


template <typename Type>
class BoxSet<Type>::const_iterator{
public:
    bool operator==(const iterator& iter) const{ return m_iter == iter.m_iter; }

    const_iterator(iterator iter)
        : m_iter(iter.m_iter)
    {}
    const_iterator operator++(){
        ++m_iter;
        return *this;
    }
    auto operator*() const{ return *m_iter; }
    auto operator*(){ return *m_iter; }
    auto operator->() const{ return m_iter.operator->(); }
    auto operator->(){ return m_iter.operator->(); }

    axis_iterator iterator_min_x() const{
        return m_iter->second.m_min_x;
    }
    axis_iterator iterator_max_x() const{
        return m_iter->second.m_max_x;
    }
    axis_iterator iterator_min_y() const{
        return m_iter->second.m_min_y;
    }
    axis_iterator iterator_max_y() const{
        return m_iter->second.m_max_y;
    }

private:
    friend class BoxSet<Type>;

    typename std::multimap<Type, Entry>::iterator m_iter;

    const_iterator(typename std::multimap<Type, Entry>::iterator iter)
        : m_iter(iter)
    {}
};





//
//  Implementations
//



template <typename Type>
inline typename BoxSet<Type>::const_iterator BoxSet<Type>::cbegin() const{
    return m_boxes.cbegin();
}
template <typename Type>
inline typename BoxSet<Type>::const_iterator BoxSet<Type>::begin () const{
    return m_boxes.begin();
}
template <typename Type>
inline typename BoxSet<Type>::iterator BoxSet<Type>::begin(){
    return m_boxes.begin();
}

template <typename Type>
inline typename BoxSet<Type>::const_iterator BoxSet<Type>::cend() const{
    return m_boxes.cend();
}
template <typename Type>
inline typename BoxSet<Type>::const_iterator BoxSet<Type>::end() const{
    return m_boxes.end();
}
template <typename Type>
inline typename BoxSet<Type>::iterator BoxSet<Type>::end(){
    return m_boxes.end();
}



template <typename Type>
size_t BoxSet<Type>::get_key(const Rectangle<Type>& box){
    //  Basically a simple hash function that preserves some sort of visual
    //  order if everything is small.
    return box.min_x + (box.min_y << 32) + (box.min_y >> 32);
}

template <typename Type>
std::string BoxSet<Type>::dump() const{
    std::string str = "BoxSet: ";
    if (m_boxes.empty()){
        str += "(empty)";
        str += "\r\n";
        return str;
    }
    str += std::to_string(m_boxes.size()) + " items\r\n";
    for (const auto& item : m_boxes){
        str += "    {" + std::to_string(item.second.m_box.min_x);
        str += "-" + std::to_string(item.second.m_box.max_x);
        str += ", " + std::to_string(item.second.m_box.min_y);
        str += "-" + std::to_string(item.second.m_box.max_y);
        str += "}\r\n";
    }
    return str;
}

template <typename Type>
typename BoxSet<Type>::iterator BoxSet<Type>::insert(const Rectangle<Type>& box){
    auto iter = m_boxes.end();
    auto min_x = m_min_x.end();
    auto max_x = m_max_x.end();
    auto min_y = m_min_y.end();
    auto max_y = m_max_y.end();
    try{
        iter = m_boxes.emplace(get_key(box), box);
        min_x = m_min_x.emplace(box.min_x, &iter->second);
        max_x = m_max_x.emplace(box.max_x, &iter->second);
        min_y = m_min_y.emplace(box.min_y, &iter->second);
        max_y = m_max_y.emplace(box.max_y, &iter->second);
        iter->second.m_min_x = min_x;
        iter->second.m_max_x = max_x;
        iter->second.m_min_y = min_y;
        iter->second.m_max_y = max_y;
    }catch (...){
        if (iter != m_boxes.end()){ m_boxes.erase(iter); }
        if (min_x != m_min_x.end()){ m_min_x.erase(min_x); }
        if (max_x != m_max_x.end()){ m_max_x.erase(max_x); }
        if (min_y != m_min_y.end()){ m_min_y.erase(min_x); }
        if (max_y != m_max_y.end()){ m_max_y.erase(max_x); }
        throw;
    }
    return iter;
}
template <typename Type>
typename BoxSet<Type>::iterator BoxSet<Type>::erase(iterator item){
    m_min_x.erase(item.iterator_min_x());
    m_max_x.erase(item.iterator_max_x());
    m_min_y.erase(item.iterator_min_y());
    m_max_y.erase(item.iterator_max_y());
    return m_boxes.erase(item.m_iter);
}
template <typename Type>
typename BoxSet<Type>::axis_iterator BoxSet<Type>::erase(axis_iterator item){
    axis_iterator next = item;
    ++next;

    size_t key = get_key(*item->second);
    auto iter = m_boxes.find(key);
    Entry* entry = item->second;

    //  Find element in the boxes map.
    while (true){
        if (iter == m_boxes.end()){
            throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "axis_iterator points in invalid box.");
        }
        if (&iter->second == entry){
            break;
        }
        ++iter;
    }

    //  Remove it.
    erase(iter);

    return next;
}


#if 0
template <typename Type>
template <typename Func>
void BoxSet<Type>::iterate_in_or_on(Type x, Type y, Func&& lambda){
    axis_iterator iter = lower_bound_min_x(x);

    //  Iterate for every x within
    while (iter == end_min_x() && iter->first <= x){
        Entry& entry = *iter->second;

        ++iter;
        if (entry.m_min_y <= y && y <= entry.m_min_y){
            lambda();
        }
    }
}
#endif







}
#endif
