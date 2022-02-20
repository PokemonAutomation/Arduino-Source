/*  Pokemon Name List Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QJsonValue>
#include "Common/Compiler.h"
#include "CommonFramework/Globals.h"
#include "Pokemon/Resources/Pokemon_PokemonSlugs.h"
#include "Pokemon_NameSelectWidget.h"
#include "Pokemon_NameListOption.h"

namespace PokemonAutomation{
namespace Pokemon{



class PokemonNameListRow : public EditableTableRow{
public:
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
        NameSelectWidget* box = new NameSelectWidget(parent, NATIONAL_DEX_SLUGS(), m_slug);
        box->connect(
            box, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            box, [&, box](int index){
                m_slug = box->slug();
            }
        );
        return {box};
    }

private:
    std::string m_slug;
};

class PokemonNameListFactory : public EditableTableFactory{
public:
    virtual QStringList make_header() const override{
        QStringList list;
        list << STRING_POKEMON;
        return list;
    }
    virtual std::unique_ptr<EditableTableRow> make_row() const override{
        return std::unique_ptr<EditableTableRow>(new PokemonNameListRow());
    }

    static const PokemonNameListFactory& instance(){
        static PokemonNameListFactory factory;
        return factory;
    }
};




PokemonNameList::PokemonNameList(QString label)
    : EditableTableOption(
        std::move(label),
        PokemonNameListFactory::instance(), true
    )
{}

const std::string& PokemonNameList::operator[](size_t index) const{
    const EditableTableRow& row = EditableTableOption::operator[](index);
    return static_cast<const PokemonNameListRow&>(row);
}



}
}
