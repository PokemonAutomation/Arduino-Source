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
    Program_JsonFile(QString category, const QString& filepath);
    Program_JsonFile(QString category, const QJsonObject& obj);

    virtual QString check_validity() const override;
    virtual void restore_defaults() override;

    virtual QJsonArray parameters_json() const override;
    virtual std::string parameters_cpp() const override;

    virtual QWidget* make_options_body(QWidget& parent) override;

private:
    std::vector<std::unique_ptr<ConfigItem>> m_options;
};


}
