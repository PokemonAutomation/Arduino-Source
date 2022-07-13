/*  Switch Date Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *      This option is thread-safe.
 *
 */

#ifndef PokemonAutomation_SwitchDateBaseOption_H
#define PokemonAutomation_SwitchDateBaseOption_H

#include <QDate>
#include "Common/Cpp/SpinLock.h"

namespace PokemonAutomation{
    class JsonValue;
namespace NintendoSwitch{


class SwitchDateBaseOption{
public:
    SwitchDateBaseOption(
        std::string label,
        QDate default_value
    );

    const std::string& label() const{ return m_label; }

    operator QDate() const;
    QDate get() const;
    std::string set(QDate x);

    std::string check_validity() const;
    std::string check_validity(QDate x) const;
    void restore_defaults();

    void load_default(const JsonValue& json);
    void load_current(const JsonValue& json);
    JsonValue write_default() const;
    JsonValue write_current() const;


protected:
    const std::string m_label;
    QDate m_default;

    mutable SpinLock m_lock;
    QDate m_current;
};



}
}
#endif

