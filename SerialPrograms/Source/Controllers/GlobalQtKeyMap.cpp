/*  Global Qt Key Map
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QKeyEvent>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "GlobalQtKeyMap.h"

namespace PokemonAutomation{



void QtKeyMap::record(const QKeyEvent& event){
    uint32_t native_key = event.nativeVirtualKey();
    if (native_key == 0){
        return;
    }
    Qt::Key qkey = (Qt::Key)event.key();
    WriteSpinLock lg(m_lock);
    m_map[native_key].insert(qkey);
}

const std::set<Qt::Key>& QtKeyMap::get_QtKeys(uint32_t native_key) const{
    ReadSpinLock lg(m_lock);
    auto iter = m_map.find(native_key);
    if (iter == m_map.end()){
        throw InternalProgramError(
            nullptr, PA_CURRENT_FUNCTION,
            "Attempted to look up unrecognized native keyboard key: " + std::to_string(native_key)
        );
    }
    return iter->second;
}



}
