/*  Program from JSON File.
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <memory>
#include <vector>
#include "Options/ConfigItem.h"
#include "Program.h"

namespace PokemonAutomation{


class Program_JsonFile : public Program{
public:
    Program_JsonFile(std::string category, const std::string& filepath);
    Program_JsonFile(std::string category, const JsonObject& obj);

    virtual QString check_validity() const override;
    virtual void restore_defaults() override;

    virtual JsonArray parameters_json() const override;
    virtual std::string parameters_cpp() const override;

    virtual QWidget* make_options_body(QWidget& parent) override;

private:
    std::vector<std::unique_ptr<ConfigItem>> m_options;
};


}
