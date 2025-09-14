/*  Global Qt Key Map
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Controllers_GlobalQtKeyMap_H
#define PokemonAutomation_Controllers_GlobalQtKeyMap_H

#include <set>
#include <map>
#include <QKeyEvent>
#include "Common/Cpp/Concurrency/SpinLock.h"

namespace PokemonAutomation{




class QtKeyMap{
public:
    struct QtKey{
        Qt::Key key;
        bool keypad;

        QtKey(Qt::Key key, bool keypad = false)
            : key(key)
            , keypad(keypad)
        {}

        bool operator<(const QtKey& x) const{
            if (keypad != x.keypad){
                return x.keypad;
            }
            return key < x.key;
        }
    };

public:
    static QtKeyMap& instance(){
        static QtKeyMap map;
        return map;
    }

    void record(const QKeyEvent& event);

    std::set<QtKey> get_QtKeys(uint32_t native_key) const;


private:
    QtKeyMap() = default;


private:
    mutable SpinLock m_lock;
    //  Map of native key to Qt's key ID.
    std::map<uint32_t, std::set<QtKey>> m_map;
};



}
#endif
