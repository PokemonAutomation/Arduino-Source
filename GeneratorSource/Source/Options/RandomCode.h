/*  RandomCode
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_RandomCode_H
#define PokemonAutomation_RandomCode_H

#include <QLabel>
#include "ConfigItem.h"

namespace PokemonAutomation{


class RandomCode : public ConfigItem{
public:
    static const QString OPTION_TYPE;
    static const QString JSON_DECLARATION_RANDOM;
    static const QString JSON_DECLARATION_CODE;
    static const QString JSON_DIGITS;
    static const QString JSON_DEFAULT_RANDOM;
    static const QString JSON_DEFAULT_CODE;
    static const QString JSON_CURRENT_RANDOM;
    static const QString JSON_CURRENT_CODE;

public:
    RandomCode(const QJsonObject& obj);

    virtual const QString& type() const override{ return OPTION_TYPE; }
    virtual bool is_valid() const override;
    virtual void restore_defaults() override;

    virtual QJsonObject to_json() const override;
    virtual std::string to_cpp() const override;

    virtual QWidget* make_ui(QWidget& parent) override;

private:
    friend class RandomCodeUI;
    QString m_declaration_random;
    QString m_declaration_code;
    size_t m_digits;
    size_t m_default_random;
    QString m_default_code;
    size_t m_current_random;
    QString m_current_code;
};



class RandomCodeUI : public QWidget{
public:
    RandomCodeUI(QWidget& parent, RandomCode& value, const QString& label);
    virtual ~RandomCodeUI();

private:
    QString sanitized_code(const QString& text) const;
    QString random_code_string() const;
    void update_labels();

private:
    RandomCode& m_value;
    QLabel* m_label_code;
    QLabel* m_under_text;
};


}
#endif
