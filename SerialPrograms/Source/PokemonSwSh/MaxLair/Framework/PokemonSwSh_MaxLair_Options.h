/*  Max Lair Options
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MaxLair_Options_H
#define PokemonAutomation_PokemonSwSh_MaxLair_Options_H

#include "Common/Qt/AutoHeightTable.h"
#include "CommonFramework/Language.h"
#include "CommonFramework/Options/BatchOption.h"
#include "CommonFramework/Options/EnumDropdownOption.h"
#include "CommonFramework/Options/RandomCodeOption.h"
#include "CommonFramework/OCR/LanguageOptionOCR.h"
#include "Pokemon/Options/Pokemon_BallSelectOption.h"
#include "Pokemon/Options/Pokemon_BallSelectWidget.h"
#include "NintendoSwitch/Options/TimeExpressionOption.h"
#include "PokemonSwSh/Options/PokemonSwSh_AutoHostNotification.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{
using namespace Pokemon;


extern const QString MODULE_NAME;
extern const std::chrono::milliseconds INFERENCE_RATE;


class MaxLairConsoleOptions : public GroupOption{
public:
    MaxLairConsoleOptions(
        QString label,
        const LanguageSet& languages,
        bool enable_boss_ball
    );

    OCR::LanguageOCR language;
    PokemonBallSelect normal_ball;
    PokemonBallSelect boss_ball;
};


enum class HostingMode{
    NOT_HOSTING,
    HOST_LOCALLY,
    HOST_ONLINE,
};

class HostingSettings : public GroupOption{
public:
    HostingSettings();

    EnumDropdownOption MODE;
    RandomCodeOption RAID_CODE;
    TimeExpressionOption<uint16_t> LOBBY_WAIT_DELAY;
    TimeExpressionOption<uint16_t> CONNECT_TO_INTERNET_DELAY;

    AutoHostNotificationOption NOTIFICATIONS;
};




class EndBattleDecider{
public:
    enum class CatchAction{
        STOP_PROGRAM,
        CATCH,
        DONT_CATCH,
    };
    enum class EndAdventureAction{
        STOP_PROGRAM,
        TAKE_NON_BOSS_SHINY_AND_CONTINUE,
        RESET,
    };

public:
    virtual CatchAction catch_boss_action() const = 0;
    virtual const std::string& normal_ball(
        size_t console_index
    ) const = 0;
    virtual const std::string& boss_ball(
        size_t console_index, const std::string& boss_slug
    ) const = 0;
    virtual EndAdventureAction end_adventure_action(
        size_t console_index
    ) const = 0;
    virtual EndAdventureAction end_adventure_action(
        size_t console_index, const std::string& boss_slug, bool shiny
    ) const = 0;
};






enum class BossAction{
    CATCH_AND_STOP_PROGRAM,
    CATCH_AND_STOP_IF_SHINY,
    DO_NOT_CATCH_AND_CONTINUE,
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

class BossActionOptionUI : public ConfigOptionUI, public QWidget{
public:
    BossActionOptionUI(QWidget& parent, BossActionOption& value);
    virtual QWidget* widget() override{ return this; }
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
