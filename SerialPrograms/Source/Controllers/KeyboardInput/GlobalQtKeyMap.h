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
    static QtKeyMap& instance(){
        static QtKeyMap map;
        return map;
    }

    void record(const QKeyEvent& event);

    std::set<Qt::Key> get_QtKeys(uint32_t native_key) const;


private:
    QtKeyMap() = default;


private:
    mutable SpinLock m_lock;
    //  Map of native key to Qt's key ID.
    std::map<uint32_t, std::set<Qt::Key>> m_map;
};



}
#endif
