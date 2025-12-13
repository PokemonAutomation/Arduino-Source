/*  Global Qt Key Map
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QKeyEvent>
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "GlobalQtKeyMap.h"

namespace PokemonAutomation{



void QtKeyMap::record(const QKeyEvent& event){
    int qt_key = event.key();
    uint32_t native_key = event.nativeVirtualKey();
    if (native_key == 0 && (qt_key == 0 || qt_key == Qt::Key_unknown)){
        // both native key and Qt key event is 0 or unknown, meaning we really don't know
        // what key event it is. Skip
        return;
    }
    
    Qt::Key qkey = (Qt::Key)qt_key;
    WriteSpinLock lg(m_lock);
    m_map[native_key].insert(
        QtKey{
            qkey,
            (event.modifiers() & Qt::KeypadModifier) != 0
        }
    );
}

std::set<QtKeyMap::QtKey> QtKeyMap::get_QtKeys(uint32_t native_key) const{
    ReadSpinLock lg(m_lock);
    auto iter = m_map.find(native_key);
    if (iter != m_map.end()){
        return iter->second;
    }
    return {};
}



}
