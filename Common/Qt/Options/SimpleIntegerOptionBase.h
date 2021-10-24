/*  Simple Integer Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *      This option is thread-safe.
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
        QString label,
        Type min_value,
        Type max_value,
        Type default_value
    );

    const QString& label() const{ return m_label; }

    operator Type() const{ return m_current.load(std::memory_order_relaxed); }
    Type get() const{ return m_current.load(std::memory_order_relaxed); }
    QString set(Type x);

    QString check_validity() const;
    QString check_validity(Type x) const;
    void restore_defaults();

    void load_default(const QJsonValue& json);
    void load_current(const QJsonValue& json);
    QJsonValue write_default() const;
    QJsonValue write_current() const;


protected:
    const QString m_label;
    const Type m_min_value;
    const Type m_max_value;
    Type m_default;
    std::atomic<Type> m_current;
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
