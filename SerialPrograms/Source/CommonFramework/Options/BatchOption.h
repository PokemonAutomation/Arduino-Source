/*  Batch Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *      "GroupOption::enabled" is thread-safe.
 *
 */

#ifndef PokemonAutomation_BatchOption_H
#define PokemonAutomation_BatchOption_H

#include <atomic>
#include <QVBoxLayout>
#include "ConfigOption.h"

namespace PokemonAutomation{


class BatchOption : public ConfigOption{
public:
//    BatchOption();
    void add_option(ConfigOption& option, QString serialization_string);

#define PA_ADD_OPTION(x)    add_option(x, #x)
#define PA_ADD_DIVIDER(x)   add_option(x, "")


public:
    virtual void load_json(const QJsonValue& json) override;
    virtual QJsonValue to_json() const override;

    QString check_validity() const override;
    virtual void restore_defaults() override;

    virtual ConfigOptionUI* make_ui(QWidget& parent) override;


protected:
    friend class BatchOptionUI;
    friend class GroupOptionUI;
    std::vector<std::pair<ConfigOption*, QString>> m_options;
};

class BatchOptionUI : public QWidget, public ConfigOptionUI{
public:
    BatchOptionUI(QWidget& parent, BatchOption& value);

    virtual QWidget* widget() override{ return this; }
    virtual void restore_defaults() override;

protected:
    BatchOption& m_value;
private:
    std::vector<ConfigOptionUI*> m_options;
};
inline ConfigOptionUI* BatchOption::make_ui(QWidget& parent){
    return new BatchOptionUI(parent, *this);
}



class GroupOption : public BatchOption{
public:
    GroupOption(
        QString label,
        bool toggleable = false,
        bool enabled = true
    );

    bool enabled() const;

    virtual void load_json(const QJsonValue& json) override;
    virtual QJsonValue to_json() const override;

    virtual ConfigOptionUI* make_ui(QWidget& parent) override;

public:
    //  Callbacks
    virtual void on_set_enabled(bool enabled){}

private:
    friend class GroupOptionUI;
    const QString m_label;
    const bool m_toggleable;

    std::atomic<bool> m_enabled;
};

class GroupOptionUI : public QWidget, public ConfigOptionUI{
public:
    GroupOptionUI(QWidget& parent, GroupOption& value);

    void set_options_enabled(bool enabled);

    virtual QWidget* widget() override{ return this; }
    virtual void restore_defaults() override;


public:
    //  Callbacks
    virtual void on_set_enabled(bool enabled){}

private:
    virtual void mouseDoubleClickEvent(QMouseEvent* event) override;

protected:
    GroupOption& m_value;
private:
    QWidget* m_placeholder;
    std::vector<ConfigOptionUI*> m_options;
    bool m_expanded = true;
protected:
    QVBoxLayout* m_options_layout;
};


}
#endif
