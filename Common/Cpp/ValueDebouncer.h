/*  Value Debouncer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *      This class is used to debounce a resizing loop in Qt with widgets of
 *  fixed aspect ratio.
 *
 *  When resizing a QScrollArea's width, the height of the contents may change.
 *  If this height changes in a way that adds/removes the scroll bar, the width
 *  will change again. If this 2nd change then toggles the scroll bar again,
 *  you get an infinite loop.
 *
 *  This class is used to help detect these infinite loops and stop the resizing.
 *
 */

#ifndef PokemonAutomation_ValueDebouncer_H
#define PokemonAutomation_ValueDebouncer_H

#include <deque>
#include <set>

namespace PokemonAutomation{


template <typename Type>
class ValueDebouncer{
public:
    ValueDebouncer(size_t history = 10)
        : m_max_history(history)
    {}

    void clear(){
        m_history.clear();
        m_recent.clear();
    }

    bool check(Type value){
        auto iter = m_recent.find(value);
        if (iter != m_recent.end()){
            return false;
        }
        m_history.push_back(value);
        try{
            m_recent.insert(value);
        }catch (...){
            m_history.pop_back();
            throw;
        }
        return true;
    }

private:
    size_t m_max_history;
    std::deque<Type> m_history;
    std::set<Type> m_recent;
};



}
#endif
