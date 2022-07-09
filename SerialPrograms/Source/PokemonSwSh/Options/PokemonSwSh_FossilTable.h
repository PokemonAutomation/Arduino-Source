/*  Fossil Table
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_FossilTable_H
#define PokemonAutomation_PokemonSwSh_FossilTable_H

#include "Common/Qt/Options/EditableTable/EditableTableBaseOption.h"
#include "Common/Qt/Options/FossilTableBaseOption.h"
#include "CommonFramework/Options/ConfigOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{



class FossilTableOption : public ConfigOption{
public:
    FossilTableOption();
    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

    size_t size() const;
    const FossilGame& operator[](size_t index) const;

    virtual QString check_validity() const override;
    virtual void restore_defaults() override;

    virtual ConfigWidget* make_ui(QWidget& parent) override;

private:
    friend class FossilTableWidget;
    FossilGameOptionFactory m_factory;
    EditableTableBaseOption m_table;
};





}
}
}
#endif
