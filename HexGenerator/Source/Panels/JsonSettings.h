/*  Settings from JSON File.
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <memory>
#include <vector>
#include "Options/ConfigItem.h"
#include "ConfigSet.h"

namespace PokemonAutomation{
    class JsonArray;
namespace HexGenerator{



class Settings_JsonFile : public ConfigSet{
public:
    ~Settings_JsonFile();
    Settings_JsonFile(std::string category, const std::string& filepath);
    Settings_JsonFile(std::string category, const JsonObject& obj);

    virtual std::string check_validity() const override;
    virtual void restore_defaults() override;

    virtual JsonArray options_json() const override;
    virtual std::string options_cpp() const override;

    virtual QWidget* make_options_body(QWidget& parent) override;

private:
    std::vector<std::unique_ptr<ConfigItem>> m_options;
    std::vector<QWidget*> m_widgets;
};


}
}
