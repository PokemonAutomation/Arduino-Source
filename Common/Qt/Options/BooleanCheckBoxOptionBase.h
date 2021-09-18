/*  Boolean Check Box Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_BooleanCheckBoxOptionBase_H
#define PokemonAutomation_BooleanCheckBoxOptionBase_H

#include <QJsonValue>
#include <QCheckBox>

namespace PokemonAutomation{


class BooleanCheckBoxOptionBase{
public:
    BooleanCheckBoxOptionBase(
        bool& backing,
        QString label,
        bool default_value
    );
    BooleanCheckBoxOptionBase(
        QString label,
        bool default_value
    );

    void load_default(const QJsonValue& json);
    void load_current(const QJsonValue& json);
    QJsonValue write_default() const;
    QJsonValue write_current() const;

    operator bool() const{ return m_current; }
    bool value() const{ return m_current; }

    void restore_defaults();

private:
    friend class BooleanCheckBoxOptionBaseUI;
    const QString m_label;
    bool m_default;
    bool& m_current;
    bool m_backing;
};


class BooleanCheckBoxOptionBaseUI : public QWidget{
public:
    BooleanCheckBoxOptionBaseUI(QWidget& parent, BooleanCheckBoxOptionBase& value);
    void restore_defaults();

private:
    BooleanCheckBoxOptionBase& m_value;
    QCheckBox* m_box;
};

}
#endif
