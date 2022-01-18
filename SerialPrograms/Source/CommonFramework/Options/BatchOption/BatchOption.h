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
#include <vector>
#include <QString>
#include "CommonFramework/Options/ConfigOption.h"

namespace PokemonAutomation{

// A ConfigOption that groups one or more options.
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

    virtual ConfigWidget* make_ui(QWidget& parent) override;


protected:
    friend class BatchWidget;
    friend class GroupWidget;
    std::vector<std::pair<ConfigOption*, QString>> m_options;
};




}
#endif
