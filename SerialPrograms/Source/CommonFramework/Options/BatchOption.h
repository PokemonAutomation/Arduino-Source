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
#include <QGroupBox>
#include "ConfigOption.h"

namespace PokemonAutomation{


class BatchOption : public ConfigOption{
public:
//    BatchOption();
    void add_option(ConfigOption& option, QString serialization_string);

#define PA_ADD_OPTION(x)    add_option(x, #x)
#define PA_ADD_STATIC(x)    add_option(x, "")


public:
    virtual void load_json(const QJsonValue& json) override;
    virtual QJsonValue to_json() const override;

    QString check_validity() const override;
    virtual void restore_defaults() override;
    virtual void reset_state() override;

    virtual ConfigOptionUI* make_ui(QWidget& parent) override;


protected:
    friend class BatchOptionUI;
    friend class GroupOptionUI;
    std::vector<std::pair<ConfigOption*, QString>> m_options;
};

class BatchOptionUI : public QWidget, public ConfigOptionUI{
public:
    BatchOptionUI(QWidget& parent, BatchOption& value);

    virtual void restore_defaults() override;
    virtual void update_visibility() override;

protected:
    BatchOption& m_value;
    std::vector<ConfigOptionUI*> m_options;
};



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

    virtual void restore_defaults() override;

    virtual ConfigOptionUI* make_ui(QWidget& parent) override;

public:
    //  Callbacks
    virtual void on_set_enabled(bool enabled){}

private:
    friend class GroupOptionUI;
    const QString m_label;
    const bool m_toggleable;
    const bool m_default_enabled;
    std::atomic<bool> m_enabled;
};

class GroupOptionUI : public QWidget, public ConfigOptionUI{
public:
    GroupOptionUI(QWidget& parent, GroupOption& value);

    void set_options_enabled(bool enabled);

    virtual void restore_defaults() override;
    virtual void update_visibility() override;


public:
    //  Callbacks
    virtual void on_set_enabled(bool enabled){}

private:
    virtual void mouseDoubleClickEvent(QMouseEvent* event) override;

protected:
    GroupOption& m_value;
    QGroupBox* m_group_box;
    QWidget* m_expand_text;
    QWidget* m_options_holder;
    std::vector<ConfigOptionUI*> m_options;
    bool m_expanded = true;
    QVBoxLayout* m_options_layout;
};


}
#endif
