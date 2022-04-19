/*  Pokemon Name List Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QJsonValue>
#include "Common/Compiler.h"
#include "CommonFramework/Globals.h"
#include "Pokemon/Resources/Pokemon_PokemonSlugs.h"
#include "PokemonSwSh/Resources/PokemonSwSh_PokemonIcons.h"
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

    virtual void load_json(const QJsonValue& json) override{
        QString value = json.toString();
        m_slug = value.toStdString();
    }
    virtual QJsonValue to_json() const override{
        return QString::fromStdString(m_slug);
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
            m_slug
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




PokemonNameListFactory::PokemonNameListFactory(const std::map<std::string, QIcon>& icons)
    : m_icons(icons)
    , m_slug_list(NATIONAL_DEX_SLUGS())
{}
PokemonNameListFactory::PokemonNameListFactory(const std::map<std::string, QIcon>& icons, std::vector<std::string> slug_list)
    : m_icons(icons)
    , m_slug_list(std::move(slug_list))
{}
QStringList PokemonNameListFactory::make_header() const{
    QStringList list;
    list << STRING_POKEMON;
    return list;
}
std::unique_ptr<EditableTableRow> PokemonNameListFactory::make_row() const{
    return std::unique_ptr<EditableTableRow>(new PokemonNameListRow(*this));
}




PokemonNameList::PokemonNameList(QString label, const std::map<std::string, QIcon>& icons)
    : PokemonNameListFactory(icons)
    , EditableTableOption(std::move(label), *this)
{}
PokemonNameList::PokemonNameList(QString label, const std::map<std::string, QIcon>& icons, std::vector<std::string> slug_list)
    : PokemonNameListFactory(icons, std::move(slug_list))
    , EditableTableOption(std::move(label), *this)
{}

const std::string& PokemonNameList::operator[](size_t index) const{
    const EditableTableRow& row = EditableTableOption::operator[](index);
    return static_cast<const PokemonNameListRow&>(row);
}



}
}
