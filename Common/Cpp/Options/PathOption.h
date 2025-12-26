/*  Path Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Options_PathOption_H
#define PokemonAutomation_Options_PathOption_H

#include "Common/Cpp/Containers/Pimpl.h"
#include "Common/Cpp/Options/ConfigOption.h"

namespace PokemonAutomation{


class PathCell : public ConfigOptionImpl<PathCell>{
public:
    ~PathCell();
    PathCell(
        LockMode lock_while_program_is_running,
        std::string default_value,
        std::string filter_string, // e.g. "*.json"
        std::string placeholder_text
    );

    const std::string& placeholder_text() const;
    const std::string default_value() const;
    const std::string& filter_string() const;


    bool is_locked() const;
    void set_locked(bool locked);

    operator std::string() const;
    void set(std::string x);

    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

    virtual void restore_defaults() override;


protected:
    virtual void sanitize(std::string& str){}


private:
    struct Data;
    Pimpl<Data> m_data;
};


class PathOption : public ConfigOptionImpl<PathOption, PathCell>{
public:
    PathOption(
        std::string label,
        LockMode lock_while_program_is_running,
        std::string default_value,
        std::string filter_string,
        std::string placeholder_text
    );

    const std::string& label() const{ return m_label; }


private:
    const std::string m_label;
};




}
#endif

