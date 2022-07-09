/*  Berry Selector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QComboBox>
#include <QString>
#include "Common/Compiler.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "Pokemon/Options/Pokemon_BerrySelectWidget.h"
#include "Pokemon/Resources/Pokemon_BerryNames.h"
#include "PokemonBDSP_BerrySelector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{

using namespace Pokemon;

BerrySelectorRow::BerrySelectorRow()
{
    berry_slug = BERRY_SLUGS()[0];
}
void BerrySelectorRow::load_json(const JsonValue& json){
    const JsonObject* obj = json.get_object();
    if (obj == nullptr){
        return;
    }
    obj->read_string(berry_slug, "Berry");
}
JsonValue BerrySelectorRow::to_json() const{
    JsonObject obj;
    obj["Berry"] = berry_slug;
    return obj;
}
std::unique_ptr<EditableTableRow> BerrySelectorRow::clone() const{
    return std::unique_ptr<EditableTableRow>(new BerrySelectorRow(*this));
}
std::vector<QWidget*> BerrySelectorRow::make_widgets(QWidget& parent){
    std::vector<QWidget*> widgets = { make_berry_box(parent) };
    return widgets;
}
QWidget* BerrySelectorRow::make_berry_box(QWidget& parent){
    auto box = new BerrySelectWidget(parent, BERRY_SLUGS(), berry_slug);
    box->connect(
        box, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
        box, [&](int index){
            if (index < 0){
                index = 0;
            }
            berry_slug = BERRY_SLUGS()[index];
        }
    );
    return box;
}


QStringList BerrySelectorOptionFactory::make_header() const{
    QStringList list;
    list << "Berry";
    return list;
}
std::unique_ptr<EditableTableRow> BerrySelectorOptionFactory::make_row() const{
    return std::unique_ptr<EditableTableRow>(new BerrySelectorRow());
}



std::vector<std::unique_ptr<EditableTableRow>> BerrySelectorOption::make_defaults() const{
    std::vector<std::unique_ptr<EditableTableRow>> ret;
    ret.emplace_back(std::unique_ptr<BerrySelectorRow>(new BerrySelectorRow()));
    return ret;
}

BerrySelectorOption::BerrySelectorOption(QString label)
    : m_table(
        std::move(label), m_factory, make_defaults()
    )
{}

void BerrySelectorOption::load_json(const JsonValue& json){
    m_table.load_json(json);
}
JsonValue BerrySelectorOption::to_json() const{
    return m_table.to_json();
}
void BerrySelectorOption::restore_defaults(){
    m_table.restore_defaults();
}
ConfigWidget* BerrySelectorOption::make_ui(QWidget& parent){
    return m_table.make_ui(parent);
}

bool BerrySelectorOption::find_berry(const std::string& berry_slug) const{
    for (size_t c = 0; c < m_table.size(); c++){
        const BerrySelectorRow& berry_row = static_cast<const BerrySelectorRow&>(m_table[c]);
        if (berry_row.berry_slug == berry_slug) {
            return true;
        }
    }
    return false;
}

std::vector<std::string> BerrySelectorOption::selected_berries() const{
    std::vector<std::string> slugs;
    for (size_t c = 0; c < m_table.size(); c++){
        const BerrySelectorRow& berry_row = static_cast<const BerrySelectorRow&>(m_table[c]);
        slugs.push_back(berry_row.berry_slug);
    }
    return slugs;
}




}
}
}
