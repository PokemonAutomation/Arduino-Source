/*  Encounter Filter
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_EncounterFilter_H
#define PokemonAutomation_PokemonSwSh_EncounterFilter_H

#include <QComboBox>
#include <QPushButton>
#include <QTableWidget>
#include "CommonFramework/Options/ConfigOption.h"
#include "Pokemon/Options/Pokemon_BallSelectWidget.h"
#include "Pokemon/Options/Pokemon_NameSelectWidget.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

using namespace Pokemon;


enum class ShinyFilter{
    ANYTHING,
    NOT_SHINY,
    ANY_SHINY,
    STAR_ONLY,
    SQUARE_ONLY,
    NOTHING,
};

enum class EncounterAction{
    StopProgram,
    RunAway,
    ThrowBalls,
    ThrowBallsAndSave,
};


struct EncounterFilterOverrides{
    EncounterFilterOverrides(bool rare_stars)
        : shininess(rare_stars ? ShinyFilter::SQUARE_ONLY : ShinyFilter::STAR_ONLY)
    {}

    EncounterAction action = EncounterAction::RunAway;
    std::string pokeball_slug = "poke-ball";

    std::string pokemon_slug;
    ShinyFilter shininess = ShinyFilter::SQUARE_ONLY;

    void load_json(const QJsonValue& json);
    QJsonValue to_json() const;
};


class EncounterFilter : public ConfigOption{
public:
    EncounterFilter(bool rare_stars, bool enable_overrides);

    ShinyFilter shiny_filter() const{ return m_shiny_filter_current; }
    const std::vector<EncounterFilterOverrides>& overrides() const{ return m_overrides; }

    virtual void load_json(const QJsonValue& json) override;
    virtual QJsonValue to_json() const override;

    virtual void restore_defaults() override;

    virtual ConfigOptionUI* make_ui(QWidget& parent) override;

private:
    friend class EncounterFilterUI;

    QString m_label;

    const bool m_rare_stars;
    const bool m_enable_overrides;

    const ShinyFilter m_shiny_filter_default;
    ShinyFilter m_shiny_filter_current;

    std::vector<EncounterFilterOverrides> m_overrides;
};


class EncounterFilterUI : public ConfigOptionUI, public QWidget{
public:
    EncounterFilterUI(QWidget& parent, EncounterFilter& value);
    virtual QWidget* widget() override{ return this; }
    virtual void restore_defaults() override;

private:
    void replace_table();
    void add_row(int row, const EncounterFilterOverrides& game, int& index_ref);
    QComboBox* make_action_box(QWidget& parent, int& row, BallSelectWidget& ball_select, EncounterAction action);
    BallSelectWidget* make_ball_select(QWidget& parent, int& row, const std::string& slug);
    NameSelectWidget* make_species_select(QWidget& parent, int& row, const std::string& slug);
    QComboBox* make_shiny_box(QWidget& parent, int& row, ShinyFilter shiniess);
    QPushButton* make_insert_button(QWidget& parent, int& row);
    QPushButton* make_remove_button(QWidget& parent, int& row);

private:
    EncounterFilter& m_value;

    QComboBox* m_shininess;
    QTableWidget* m_table;
    std::vector<std::unique_ptr<int>> m_index_table;
};


}
}
}
#endif
