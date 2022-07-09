/*  String Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *      This option is thread-safe.
 *
 */

#ifndef PokemonAutomation_StringBaseOption_H
#define PokemonAutomation_StringBaseOption_H

#include <QString>
#include "Common/Cpp/SpinLock.h"

namespace PokemonAutomation{

class JsonValue;


class StringBaseOption{
public:
    StringBaseOption(
        bool is_password,
        QString label,
        QString default_value,
        QString placeholder_text
    );

    const QString& label() const{ return m_label; }
    const QString& placeholder_text() const{ return m_placeholder_text; }
    bool is_password() const{ return m_is_password; }

    operator QString() const;
    QString get() const;
    void set(QString x);

    void restore_defaults();

    void load_default(const JsonValue& json);
    void load_current(const JsonValue& json);
    JsonValue write_default() const;
    JsonValue write_current() const;

private:
    const QString m_label;
    QString m_default;
    QString m_placeholder_text;
    bool m_is_password;

    mutable SpinLock m_lock;
    QString m_current;
};




}
#endif
