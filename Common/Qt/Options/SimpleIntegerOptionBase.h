/*  Simple Integer Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_SimpleIntegerOptionBase_H
#define PokemonAutomation_SimpleIntegerOptionBase_H

#include <QJsonValue>
#include <QLineEdit>

namespace PokemonAutomation{


template <typename Type>
class SimpleIntegerOptionBase{
public:
    SimpleIntegerOptionBase(
        Type& backing,
        QString label,
        Type min_value,
        Type max_value,
        Type default_value
    );
    SimpleIntegerOptionBase(
        QString label,
        Type min_value,
        Type max_value,
        Type default_value
    );

    void load_default(const QJsonValue& json);
    void load_current(const QJsonValue& json);
    QJsonValue write_default() const;
    QJsonValue write_current() const;

    operator Type() const{ return m_current; }
    Type value() const{ return m_current; }

    bool is_valid() const;
    void restore_defaults();

protected:
    template <typename> friend class SimpleIntegerOptionBaseUI;
    const QString m_label;
    const Type m_min_value;
    const Type m_max_value;
    Type m_default;
    Type& m_current;
    Type m_backing;
};


template <typename Type>
class SimpleIntegerOptionBaseUI : public QWidget{
public:
    SimpleIntegerOptionBaseUI(QWidget& parent, SimpleIntegerOptionBase<Type>& value);
    void restore_defaults();

private:
    SimpleIntegerOptionBase<Type>& m_value;
    QLineEdit* m_box;
};



}
#endif
