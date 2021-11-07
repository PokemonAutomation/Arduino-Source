/*  Max Lair Boss Action
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MaxLair_Options_BossAction_H
#define PokemonAutomation_PokemonSwSh_MaxLair_Options_BossAction_H

#include "Common/Qt/AutoHeightTable.h"
#include "Pokemon/Options/Pokemon_BallSelectOption.h"
#include "Pokemon/Options/Pokemon_BallSelectWidget.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{



enum class BossAction{
    CATCH_AND_STOP_PROGRAM,
    CATCH_AND_STOP_IF_SHINY,
};

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

    virtual void load_json(const QJsonValue& json) override;
    virtual QJsonValue to_json() const override;

    virtual void restore_defaults() override;

    virtual ConfigOptionUI* make_ui(QWidget& parent) override;

private:
    friend class BossActionOptionUI;

    QString m_label;
    std::vector<BossFilter> m_list;
};

class BossActionOptionUI : public QWidget, public ConfigOptionUI{
public:
    BossActionOptionUI(QWidget& parent, BossActionOption& value);
    virtual void restore_defaults() override;

private:
    QComboBox* make_action_box(QWidget& parent, int row, BallSelectWidget& ball_select, BossAction action);
    BallSelectWidget* make_ball_select(QWidget& parent, int row, const std::string& slug);


private:
    BossActionOption& m_value;
    AutoHeightTableWidget* m_table;
};




}
}
}
}
#endif
