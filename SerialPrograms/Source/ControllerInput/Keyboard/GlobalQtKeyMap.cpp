/*  Global Qt Key Map
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QKeyEvent>
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "GlobalQtKeyMap.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{



QtKeyMap::QtKey QtKeyMap::record(const QKeyEvent& event){
    int qt_key = event.key();
    uint32_t native_key = event.nativeVirtualKey();

    QtKeyMap::QtKey ret{Qt::Key_unknown};

    if (native_key == 0 && (qt_key == 0 || qt_key == Qt::Key_unknown)){
        // both native key and Qt key event is 0 or unknown, meaning we really don't know
        // what key event it is. Skip
        return ret;
    }

    ret.key = (Qt::Key)qt_key;
    ret.keypad = (event.modifiers() & Qt::KeypadModifier) != 0;

//    cout << "key: " << ret.key << ", keypad = " << ret.keypad << endl;

    WriteSpinLock lg(m_lock, PA_CURRENT_FUNCTION);
    m_map[native_key].insert(ret);

    return ret;
}

std::set<QtKeyMap::QtKey> QtKeyMap::get_QtKeys(uint32_t native_key) const{
    ReadSpinLock lg(m_lock, PA_CURRENT_FUNCTION);
    auto iter = m_map.find(native_key);
    if (iter != m_map.end()){
        return iter->second;
    }
    return {};
}



}
