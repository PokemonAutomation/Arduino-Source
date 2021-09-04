/*  Batch Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_BatchOption_H
#define PokemonAutomation_BatchOption_H

#include "ConfigOption.h"

namespace PokemonAutomation{


class BatchOption : public ConfigOption{
public:
//    BatchOption();
    void add_option(ConfigOption& option, QString serialization_string);

#define PA_ADD_OPTION(x)    add_option(x, #x)

    virtual void load_json(const QJsonValue& json) override;
    virtual QJsonValue to_json() const override;

    bool is_valid() const override;
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
    virtual bool settings_valid() const;
    virtual void restore_defaults() override;

protected:
    BatchOption& m_value;
    std::vector<ConfigOptionUI*> m_options;
};
inline ConfigOptionUI* BatchOption::make_ui(QWidget& parent){
    return new BatchOptionUI(parent, *this);
}



class GroupOption : public BatchOption{
public:
    GroupOption(QString label);
    virtual ConfigOptionUI* make_ui(QWidget& parent) override;
private:
    friend class GroupOptionUI;
    QString m_label;
};
class GroupOptionUI : public QWidget, public ConfigOptionUI{
public:
    GroupOptionUI(QWidget& parent, GroupOption& value);

    virtual QWidget* widget() override{ return this; }
    virtual bool settings_valid() const;
    virtual void restore_defaults() override;

protected:
    BatchOption& m_value;
    std::vector<ConfigOptionUI*> m_options;
};
inline ConfigOptionUI* GroupOption::make_ui(QWidget& parent){
    return new GroupOptionUI(parent, *this);
}


}
#endif
