/*  Batch Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *      "GroupOption::enabled" is thread-safe.
 *
 */

#ifndef PokemonAutomation_BatchOption_H
#define PokemonAutomation_BatchOption_H

#include <string>
#include <vector>
#include "CommonFramework/Options/ConfigOption.h"

namespace PokemonAutomation{

// A ConfigOption that groups one or more options.
class BatchOption : public ConfigOption{
public:
//    BatchOption();
    void add_option(ConfigOption& option, std::string serialization_string);

#define PA_ADD_STATIC(x)    add_option(x, "")
#define PA_ADD_OPTION(x)    add_option(x, #x)


public:
    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

    std::string check_validity() const override;
    virtual void restore_defaults() override;
    virtual void reset_state() override;

    virtual ConfigWidget* make_ui(QWidget& parent) override;


protected:
    friend class BatchWidget;
    friend class GroupWidget;
    std::vector<std::pair<ConfigOption*, std::string>> m_options;
};




}
#endif
