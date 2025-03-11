/*  Batch Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Options_BatchOption_H
#define PokemonAutomation_Options_BatchOption_H

#include "Common/Cpp/Containers/Pimpl.h"
#include "Common/Cpp/Containers/FixedLimitVector.h"
#include "ConfigOption.h"

namespace PokemonAutomation{

// A ConfigOption that groups one or more options.
class BatchOption : public ConfigOption{
public:
    ~BatchOption();
    BatchOption(LockMode lock_while_program_is_running, bool horizontal = false);

public:
    //  This is not thread-safe with the rest of this class. You must
    //  fully initialize the class (by adding all the options it will ever have)
    //  before you start using it.
    void add_option(ConfigOption& option, std::string serialization_string);
#define PA_ADD_STATIC(x)    add_option(x, "")
#define PA_ADD_OPTION(x)    add_option(x, #x)


public:
    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

    std::string check_validity() const override;
    virtual void restore_defaults() override;
    virtual void reset_state() override;

    virtual void report_program_state(bool program_is_running) override;

    virtual ConfigWidget* make_QtWidget(QWidget& parent) override;

    bool horizontal() const;
    FixedLimitVector<ConfigOption*> options() const;


private:
    struct Data;
    Pimpl<Data> m_data;
};




}
#endif
