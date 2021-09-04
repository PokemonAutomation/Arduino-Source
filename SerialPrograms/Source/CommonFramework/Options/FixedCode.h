/*  Fixed Code Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_FixedCode_H
#define PokemonAutomation_FixedCode_H

#include <QJsonValue>
#include <QLineEdit>
#include "ConfigOption.h"

namespace PokemonAutomation{


class FixedCode : public ConfigOption{
public:
    FixedCode(
        QString label,
        size_t digits,
        QString default_value
    );
    virtual void load_json(const QJsonValue& json) override;
    virtual QJsonValue to_json() const override;

    operator const QString&() const{ return m_current; }
    const QString& value() const{ return m_current; }
    void to_str(uint8_t* code) const;

    virtual bool is_valid() const override;
    virtual void restore_defaults() override;

    virtual ConfigOptionUI* make_ui(QWidget& parent) override;

private:
    friend class FixedCodeUI;
    QString m_label;
    const size_t m_digits;
    const QString m_default;
    QString m_current;
};


class FixedCodeUI : public ConfigOptionUI, public QWidget{
public:
    FixedCodeUI(QWidget& parent, FixedCode& value);
    virtual QWidget* widget() override{ return this; }
    virtual void restore_defaults() override;

private:
    QString sanitized_code(const QString& text) const;

private:
    FixedCode& m_value;
    QLineEdit* m_box;
};


}
#endif
