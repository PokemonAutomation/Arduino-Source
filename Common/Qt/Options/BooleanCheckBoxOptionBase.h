/*  Boolean Check Box Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *      This option is thread-safe.
 *
 */

#ifndef PokemonAutomation_BooleanCheckBoxOptionBase_H
#define PokemonAutomation_BooleanCheckBoxOptionBase_H

#include <atomic>
#include <QJsonValue>
#include <QCheckBox>

namespace PokemonAutomation{


class BooleanCheckBoxOptionBase{
public:
    BooleanCheckBoxOptionBase(
        QString label,
        bool default_value
    );

    const QString& label() const{ return m_label; }

    operator bool() const{ return m_current.load(std::memory_order_relaxed); }
    bool get() const{ return m_current.load(std::memory_order_relaxed); }
    void set(bool x){ m_current.store(x, std::memory_order_relaxed); }

    void load_default(const QJsonValue& json);
    void load_current(const QJsonValue& json);
    QJsonValue write_default() const;
    QJsonValue write_current() const;

    void restore_defaults();

private:
    const QString m_label;
    bool m_default;
    std::atomic<bool> m_current;
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
