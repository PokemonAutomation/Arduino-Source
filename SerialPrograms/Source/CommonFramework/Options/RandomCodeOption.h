/*  Random Code Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_RandomCode_H
#define PokemonAutomation_RandomCode_H

#include <QJsonValue>
#include <QLabel>
#include <QLineEdit>
#include "ConfigOption.h"

namespace PokemonAutomation{


class RaidCodeOption{
public:
    RaidCodeOption();
    RaidCodeOption(size_t random_digits, QString code_string);

    size_t total_digits() const{ return m_digits; }
    size_t random_digits() const{ return m_random_digits; }
    const QString& code_string() const{ return m_code; }

    QString check_validity() const;
    bool code_enabled() const;
    bool get_code(uint8_t* code) const;

//private:
    size_t m_digits;
    size_t m_random_digits;
    QString m_code;
};

class RandomCodeOption : public ConfigOption{
public:
    RandomCodeOption();
    RandomCodeOption(QString label, size_t random_digits, QString code_string);

    virtual void load_json(const QJsonValue& json) override;
    virtual QJsonValue to_json() const override;

    bool code_enabled() const;
    size_t random_digits() const{ return m_current.random_digits(); }
    bool get_code(uint8_t* code) const;

    virtual QString check_validity() const override;
    virtual void restore_defaults() override;

    virtual ConfigOptionUI* make_ui(QWidget& parent) override;

private:
    friend class RandomCodeOptionUI;
    QString m_label;
    const RaidCodeOption m_default;
    RaidCodeOption m_current;
};


class RandomCodeOptionUI : public QWidget, public ConfigOptionUI{
public:
    RandomCodeOptionUI(QWidget& parent, RandomCodeOption& value);
    virtual void restore_defaults() override;

private:
    QString sanitized_code(const QString& text) const;
    QString random_code_string() const;
    void update_labels();

private:
    RandomCodeOption& m_value;
    QLabel* m_label_code;
    QLabel* m_under_text;
    QLineEdit* m_box_random;
    QLineEdit* m_box_code;
};


}
#endif
