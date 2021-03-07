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


class Settings_JsonFile : public ConfigSet{
public:
    Settings_JsonFile(const QString& filepath);
    Settings_JsonFile(const QJsonObject& obj);

    virtual bool is_valid() const override;
    virtual void restore_defaults() override;

    virtual QJsonArray options_json() const override;
    virtual std::string options_cpp() const override;

    virtual QWidget* make_options_body(QWidget& parent) override;

private:
    std::vector<std::unique_ptr<ConfigItem>> m_options;
};


}
