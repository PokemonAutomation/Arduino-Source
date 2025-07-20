/*  MAC Address Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <mutex>
#include "Common/Cpp/Containers/Pimpl.tpp"
#include "Common/Cpp/Json/JsonValue.h"
#include "MacAddressOption.h"

#include "Common/Qt/Options/MacAddressWidget.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{



ConfigWidget* MacAddressCell::make_QtWidget(QWidget& parent){
    return new MacAddressCellWidget(parent, *this);
}



std::string write_MAC_address(size_t length, const uint8_t* address){
    static const char HEX_DIGITS[] = "0123456789ABCDEF";
    std::string ret;
    for (size_t c = 0; c < length; c++){
        if (!ret.empty()){
            ret += "-";
        }
        ret += HEX_DIGITS[address[c] >> 4];
        ret += HEX_DIGITS[address[c] & 0xf];
    }
    return ret;
}
void parse_MAC_address(size_t length, uint8_t* address, const std::string& str){
    memset(address, 0, length);
    size_t current_index = 0;
    for (char ch : str){
        if (current_index >= 2*length){
            return;
        }
        if ('0' <= ch && ch <= '9'){
            ch -= '0';
        }else if ('a' <= ch && ch <= 'f'){
            ch += 10 - 'a';
        }else if ('A' <= ch && ch <= 'F'){
            ch += 10 - 'A';
        }else{
            continue;
        }
        address[current_index / 2] |= ch << 4*(1 - current_index % 2);
        current_index++;
    }
}



struct MacAddressCell::Data{
    mutable std::mutex m_lock;
    std::vector<uint8_t> m_current;

    Data(size_t bytes, const uint8_t* current)
        : m_current(bytes)
    {
        if (current != nullptr){
            memcpy(m_current.data(), current, bytes);
        }
    }
};



MacAddressCell::~MacAddressCell() = default;
MacAddressCell::MacAddressCell(const MacAddressCell& x)
    : ConfigOption(x)
    , m_data(CONSTRUCT_TOKEN, x.m_data->m_current.size(), x.m_data->m_current.data())
{}
void MacAddressCell::operator=(const MacAddressCell& x){
    if (bytes() != x.bytes()){
        throw InternalProgramError(
            nullptr, PA_CURRENT_FUNCTION,
            "Attempt to assign a MAC address is mismatching size."
        );
    }
    {
        std::scoped_lock<std::mutex, std::mutex> lg(m_data->m_lock, x.m_data->m_lock);
        memcpy(m_data->m_current.data(), x.m_data->m_current.data(), bytes());
    }
    report_value_changed(this);
}
MacAddressCell::MacAddressCell(
    LockMode lock_while_running,
    size_t bytes,
    uint8_t* current_value
)
    : ConfigOption(lock_while_running)
    , m_data(CONSTRUCT_TOKEN, bytes, current_value)
{}


size_t MacAddressCell::bytes() const{
    return m_data->m_current.size();
}
std::string MacAddressCell::to_string() const{
    std::lock_guard<std::mutex> lg(m_data->m_lock);
    return write_MAC_address(m_data->m_current.size(), m_data->m_current.data());
}
void MacAddressCell::current_value(uint8_t* address) const{
    std::lock_guard<std::mutex> lg(m_data->m_lock);
    memcpy(address, m_data->m_current.data(), m_data->m_current.size());
}
void MacAddressCell::set(const uint8_t* address){
    {
        std::lock_guard<std::mutex> lg(m_data->m_lock);
        if (memcmp(m_data->m_current.data(), address, m_data->m_current.size()) == 0){
            return;
        }
        memcpy(m_data->m_current.data(), address, m_data->m_current.size());
    }
    report_value_changed(this);
}
void MacAddressCell::set(const std::string& address){
    std::vector<uint8_t> new_value(m_data->m_current.size());
    parse_MAC_address(new_value.size(), new_value.data(), address);
    set(new_value.data());
}


bool MacAddressCell::operator==(const uint8_t* address) const{
    std::lock_guard<std::mutex> lg(m_data->m_lock);
    return memcmp(m_data->m_current.data(), address, m_data->m_current.size()) == 0;
}


void MacAddressCell::load_json(const JsonValue& json){
    set(json.to_string_default());
}
JsonValue MacAddressCell::to_json() const{
    return to_string();
}





}
