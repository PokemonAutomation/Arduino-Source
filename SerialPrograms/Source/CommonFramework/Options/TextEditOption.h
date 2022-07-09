/*  Text Edit
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *      This option is thread-safe.
 *
 */

#ifndef PokemonAutomation_TextEdit_H
#define PokemonAutomation_TextEdit_H

#include <QString>
#include "Common/Cpp/SpinLock.h"
#include "ConfigOption.h"

namespace PokemonAutomation{


class TextEditOption : public ConfigOption{
public:
    TextEditOption(
        QString label,
        QString default_value,
        QString placeholder_text
    );

    const QString& label() const{ return m_label; }
    const QString& placeholder_text() const{ return m_placeholder_text; }

    operator const QString&() const;
    void set(QString x);

    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

    virtual void restore_defaults() override;

    virtual ConfigWidget* make_ui(QWidget& parent) override;

private:
    const QString m_label;
    const QString m_default;
    const QString m_placeholder_text;

    mutable SpinLock m_lock;
    QString m_current;
};




}
#endif
