/*  HID Keyboard Mappings
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ControllerInput_KeyboardInput_KeyMappings_H
#define PokemonAutomation_ControllerInput_KeyboardInput_KeyMappings_H

#include <map>
#include "GlobalQtKeyMap.h"
#include "KeyboardHidButtons.h"

namespace PokemonAutomation{


struct KeyboardInputMappingKey{
    Qt::Key key;
    bool keypad;

    KeyboardInputMappingKey(Qt::Key key)
        : key(key)
        , keypad(false)
    {}
    KeyboardInputMappingKey(Qt::Key key, bool keypad)
        : key(key)
        , keypad(keypad)
    {}
};

struct KeyboardInputMappingEntry{
    KeyboardInputMappingKey key;
    KeyboardKey hid_id;
};



class KeyboardInputMappings{
public:
    KeyboardInputMappings(std::initializer_list<KeyboardInputMappingEntry> list);

    void add(const KeyboardInputMappingEntry& entry);

    KeyboardKey get(const QtKeyMap::QtKey& key) const;

private:
    struct Entry{
        KeyboardKey standard = KeyboardKey::KEY_NONE;
        KeyboardKey keypad = KeyboardKey::KEY_NONE;
    };
    std::map<Qt::Key, Entry> m_map;
};






const KeyboardInputMappings& get_keyid_to_hid_map();

const std::map<Qt::Key, std::string>& QTKEY_TO_STRING();
const std::map<KeyboardKey, std::string>& KEYBOARDKEY_TO_STRING();

const KeyboardInputMappings& KEYID_TO_HID_QWERTY();
const KeyboardInputMappings& KEYID_TO_HID_AZERTY();



}
#endif
