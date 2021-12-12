/*  Encounter Filter
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_EncounterFilter_H
#define PokemonAutomation_PokemonBDSP_EncounterFilter_H

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
namespace PokemonBDSP{

using namespace Pokemon;


enum class ShinyFilter{
    ANYTHING,
    NOT_SHINY,
    SHINY,
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
    EncounterFilterOverride(bool allow_autocatch);

    virtual void load_json(const QJsonValue& json) override;
    virtual QJsonValue to_json() const override;
    virtual std::unique_ptr<EditableTableRow> clone() const override;
    virtual std::vector<QWidget*> make_widgets(QWidget& parent) override;

private:
    QComboBox* make_action_box(QWidget& parent);
    BallSelectWidget* make_ball_select(QWidget& parent);
    NameSelectWidget* make_species_select(QWidget& parent);
    QComboBox* make_shiny_box(QWidget& parent);

    void update_ball_select();

private:
    bool m_allow_autocatch;
    BallSelectWidget* m_ball_select;
public:
    EncounterAction action = EncounterAction::RunAway;
    std::string pokeball_slug = "poke-ball";

    std::string pokemon_slug;
    ShinyFilter shininess = ShinyFilter::SHINY;
};

class EncounterFilterOptionFactory : public EditableTableFactory{
public:
    EncounterFilterOptionFactory(bool allow_autocatch);
    virtual QStringList make_header() const override;
    virtual std::unique_ptr<EditableTableRow> make_row() const override;
private:
    bool m_allow_autocatch;
};




class EncounterFilterOption : public ConfigOption{
public:
    EncounterFilterOption(bool enable_overrides, bool allow_autocatch);

    ShinyFilter shiny_filter() const{ return m_shiny_filter_current; }
    std::vector<EncounterFilterOverride> overrides() const;

    virtual void load_json(const QJsonValue& json) override;
    virtual QJsonValue to_json() const override;

    virtual void restore_defaults() override;

    virtual ConfigOptionUI* make_ui(QWidget& parent) override;

private:
    friend class EncounterFilterOptionUI;

    QString m_label;

    const bool m_enable_overrides;
    const bool m_allow_autocatch;

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
