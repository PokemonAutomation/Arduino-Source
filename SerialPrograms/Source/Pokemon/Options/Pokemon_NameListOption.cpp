/*  Pokemon Name List Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Compiler.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "CommonFramework/Globals.h"
#include "Pokemon/Pokemon_Strings.h"
#include "Pokemon_NameSelectWidget.h"
#include "Pokemon_NameListOption.h"

namespace PokemonAutomation{
namespace Pokemon{



class PokemonNameListRow : public EditableTableRow{
public:
    PokemonNameListRow(const PokemonNameListFactory& factory)
        : m_factory(factory)
    {}

    operator const std::string&() const{ return m_slug; }

    virtual void load_json(const JsonValue& json) override{
        json.read_string(m_slug);
    }
    virtual JsonValue to_json() const override{
        return m_slug;
    }

    virtual std::unique_ptr<EditableTableRow> clone() const override{
        return std::unique_ptr<EditableTableRow>(new PokemonNameListRow(*this));
    }
    virtual std::vector<QWidget*> make_widgets(QWidget& parent) override{
        using namespace Pokemon;
        NameSelectWidget* box = new NameSelectWidget(
            parent,
            m_factory.m_icons,
            m_factory.m_slug_list,
            m_slug,
            (m_factory.m_display_name_mapping ? &m_factory.m_display_name_mapping->display_names : nullptr),
            (m_factory.m_display_name_mapping ? &m_factory.m_display_name_mapping->display_name_to_slug : nullptr),
            (m_factory.m_extra_names ? &m_factory.m_extra_names->names : nullptr),
            (m_factory.m_extra_names ? &m_factory.m_extra_names->name_list : nullptr),
            (m_factory.m_extra_names ? &m_factory.m_extra_names->display_name_to_slug : nullptr)
        );
        box->connect(
            box, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            box, [&, box](int index){
                m_slug = box->slug();
            }
        );
        return {box};
    }

private:
    const PokemonNameListFactory& m_factory;
    std::string m_slug;
};




PokemonNameListFactory::PokemonNameListFactory(
    const SpriteDatabase& icons,
    std::vector<std::string> slug_list,
    const DisplayNameMapping* display_name_mapping,
    const ExtraNames* extra_names
)
    : m_icons(icons)
    , m_slug_list(std::move(slug_list))
    , m_display_name_mapping(display_name_mapping)
    , m_extra_names(extra_names)
{}

std::vector<std::string> PokemonNameListFactory::make_header() const{
    return std::vector<std::string>{
        STRING_POKEMON,
    };
}
std::unique_ptr<EditableTableRow> PokemonNameListFactory::make_row() const{
    return std::unique_ptr<EditableTableRow>(new PokemonNameListRow(*this));
}




PokemonNameList::PokemonNameList(
    std::string label,
    const SpriteDatabase& icons,
    std::vector<std::string> slug_list,
    const DisplayNameMapping* display_name_mapping,
    const ExtraNames* extra_names
)
    : PokemonNameListFactory(icons, std::move(slug_list), display_name_mapping, extra_names)
    , EditableTableOption(std::move(label), *this)
{}

const std::string& PokemonNameList::operator[](size_t index) const{
    const EditableTableRow& row = EditableTableOption::operator[](index);
    return static_cast<const PokemonNameListRow&>(row);
}



}
}
