/*  Text Edit
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_TextEdit_H
#define PokemonAutomation_TextEdit_H

#include <QTextEdit>
#include "ConfigOption.h"

namespace PokemonAutomation{


class TextEditOption : public ConfigOption{
public:
    TextEditOption(QString label, QString default_value);
    TextEditOption(QString& backing, QString label, QString default_value);

    operator const QString&() const{ return m_current; }

    virtual void load_json(const QJsonValue& json) override;
    virtual QJsonValue to_json() const override;

    virtual void restore_defaults() override;

    virtual ConfigOptionUI* make_ui(QWidget& parent) override;

private:
    friend class TextEditOptionUI;
    QString m_label;
    QString m_default;
    QString& m_current;
    QString m_backing;
};


class TextEditOptionUI : public ConfigOptionUI, public QWidget{
public:
    TextEditOptionUI(QWidget& parent, TextEditOption& value);
    virtual QWidget* widget() override{ return this; }
    virtual void restore_defaults() override;

private:
    class Box;

    TextEditOption& m_value;
    QTextEdit* m_box;
};


}
#endif
