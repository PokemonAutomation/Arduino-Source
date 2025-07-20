/*  MAC Address Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *      This option is thread-safe.
 *
 */

#ifndef PokemonAutomation_Options_MacAddressOption_H
#define PokemonAutomation_Options_MacAddressOption_H

#include "Common/Cpp/Containers/Pimpl.h"
#include "ConfigOption.h"

namespace PokemonAutomation{


std::string write_MAC_address(size_t length, const uint8_t* address);
void parse_MAC_address(size_t length, uint8_t* address, const std::string& str);


class MacAddressCell : public ConfigOption{
public:
    ~MacAddressCell();
    MacAddressCell(const MacAddressCell& x);
    void operator=(const MacAddressCell& x);
    MacAddressCell(
        LockMode lock_while_running,
        size_t bytes,
        uint8_t* current_value
    );

public:
    size_t bytes() const;
    std::string to_string() const;

    //  Buffer must be long enough.

    void current_value(uint8_t* address) const;
    void set(const uint8_t* address);
    void set(const std::string& address);

    bool operator==(const uint8_t* address) const;

    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

public:
    virtual ConfigWidget* make_QtWidget(QWidget& parent) override;

protected:
    struct Data;
    Pimpl<Data> m_data;
};






}
#endif

