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
#include "Common/Qt/AutoHeightTable.h"
#include "CommonFramework/Options/ConfigOption.h"
#include "CommonFramework/Options/EditableTableOption.h"
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


class EncounterFilterOverride : public EditableTableRow{
public:
    EncounterFilterOverride(bool rare_stars);

    virtual void load_json(const QJsonValue& json) override;
    virtual QJsonValue to_json() const override;
    virtual std::unique_ptr<EditableTableRow> clone() const override;
    virtual std::vector<QWidget*> make_widgets(QWidget& parent) override;

private:
    QComboBox* make_action_box(QWidget& parent, BallSelectWidget& ball_select);
    BallSelectWidget* make_ball_select(QWidget& parent);
    NameSelectWidget* make_species_select(QWidget& parent);
    QComboBox* make_shiny_box(QWidget& parent);

private:
    bool m_rare_stars;
public:
    EncounterAction action = EncounterAction::StopProgram;
    std::string pokeball_slug = "poke-ball";

    std::string pokemon_slug;
    ShinyFilter shininess = ShinyFilter::NOT_SHINY;
};

class EncounterFilterOptionFactory : public EditableTableFactory{
public:
    EncounterFilterOptionFactory(bool rare_stars);
    virtual QStringList make_header() const override;
    virtual std::unique_ptr<EditableTableRow> make_row() const override;
private:
    bool m_rare_stars;
};




class EncounterFilterOption : public ConfigOption{
public:
    EncounterFilterOption(bool rare_stars, bool enable_overrides);

    ShinyFilter shiny_filter() const{ return m_shiny_filter_current; }
    std::vector<EncounterFilterOverride> overrides() const;

    virtual void load_json(const QJsonValue& json) override;
    virtual QJsonValue to_json() const override;

    virtual void restore_defaults() override;

    virtual ConfigOptionUI* make_ui(QWidget& parent) override;

private:
    friend class EncounterFilterOptionUI;

    QString m_label;

    const bool m_rare_stars;
    const bool m_enable_overrides;

    const ShinyFilter m_shiny_filter_default;
    ShinyFilter m_shiny_filter_current;

    EncounterFilterOptionFactory m_factory;
    EditableTableOption m_table;
};

class EncounterFilterOptionUI : public QWidget, public ConfigOptionUI{
public:
    EncounterFilterOptionUI(QWidget& parent, EncounterFilterOption& value);

    virtual void restore_defaults() override;

private:
    EncounterFilterOption& m_value;

    QComboBox* m_shininess = nullptr;
    ConfigOptionUI* m_table = nullptr;
};




}
}
}
#endif
