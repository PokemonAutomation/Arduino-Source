/*  Max Lair Boss Action
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MaxLair_Options_BossAction_H
#define PokemonAutomation_PokemonSwSh_MaxLair_Options_BossAction_H

#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "Common/Cpp/Options/StaticTableOption.h"
#include "Common/Qt/AutoHeightTable.h"
#include "Common/Qt/Options/ConfigWidget.h"
#include "CommonFramework/Options/LabelCellOption.h"
#include "Pokemon/Options/Pokemon_BallSelectWidget.h"
#include "PokemonSwSh/Options/PokemonSwSh_BallSelectOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{

using namespace Pokemon;


enum class BossAction{
    CATCH_AND_STOP_PROGRAM,
    CATCH_AND_STOP_IF_SHINY,
};


class BossActionRow : public StaticTableRow{
public:
    BossActionRow(std::string slug, const std::string& name_slug, const std::string& sprite_slug);

    LabelCellOption pokemon;
    EnumDropdownCell<BossAction> action;
    PokemonBallSelectCell ball;
};

class BossActionTable : public StaticTableOption{
public:
    BossActionTable();
    virtual std::vector<std::string> make_header() const;
};



#if 0
struct BossFilter{
    std::string slug;
    BossAction action = BossAction::CATCH_AND_STOP_IF_SHINY;
    std::string ball = "poke-ball";

    void restore_defaults(){
        action = BossAction::CATCH_AND_STOP_IF_SHINY;
        ball = "poke-ball";
    }
};

class BossActionOption : public ConfigOption{
public:
    BossActionOption();

    const std::vector<BossFilter>& list() const { return m_list; }

    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

    virtual void restore_defaults() override;

    virtual ConfigWidget* make_ui(QWidget& parent) override;

private:
    friend class BossActionWidget;

    std::string m_label;
    std::vector<BossFilter> m_list;
};

class BossActionWidget : public QWidget, public ConfigWidget{
public:
    BossActionWidget(QWidget& parent, BossActionOption& value);

    virtual void update() override;

private:
    void redraw_table();
    QComboBox* make_action_box(QWidget& parent, int row, BallSelectWidget& ball_select, BossAction action);
    BallSelectWidget* make_ball_select(QWidget& parent, int row, const std::string& slug);


private:
    BossActionOption& m_value;
    AutoHeightTableWidget* m_table;
};
#endif



}
}
}
}
#endif
