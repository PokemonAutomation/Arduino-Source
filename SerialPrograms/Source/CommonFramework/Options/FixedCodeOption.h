/*  Fixed Code Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *      This option is thread-safe.
 *
 */

#ifndef PokemonAutomation_FixedCode_H
#define PokemonAutomation_FixedCode_H

#include <QJsonValue>
#include <QLineEdit>
#include "Common/Cpp/SpinLock.h"
#include "ConfigOption.h"

namespace PokemonAutomation{


class FixedCodeOption : public ConfigOption{
public:
    FixedCodeOption(
        QString label,
        size_t digits,
        QString default_value
    );

    operator const QString&() const;
    const QString& get() const;
    QString set(QString x);
    void to_str(uint8_t* code) const;

    virtual QString check_validity() const override;
    QString check_validity(const QString& x) const;
    virtual void restore_defaults() override;

    virtual void load_json(const QJsonValue& json) override;
    virtual QJsonValue to_json() const override;

    virtual ConfigOptionUI* make_ui(QWidget& parent) override;

private:
    friend class FixedCodeOptionUI;
    QString m_label;
    const size_t m_digits;
    const QString m_default;

    mutable SpinLock m_lock;
    QString m_current;
};


class FixedCodeOptionUI : public QWidget, public ConfigOptionUI{
public:
    FixedCodeOptionUI(QWidget& parent, FixedCodeOption& value);
    virtual void restore_defaults() override;

private:
    QString sanitized_code(const QString& text) const;

private:
    FixedCodeOption& m_value;
    QLineEdit* m_box;
};


}
#endif
