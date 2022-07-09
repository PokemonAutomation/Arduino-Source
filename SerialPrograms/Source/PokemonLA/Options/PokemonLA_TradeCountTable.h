/*  Trade Count Table
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonLA_TradeCountTable_H
#define PokemonAutomation_PokemonLA_TradeCountTable_H

#include <string>
#include <vector>
#include <QString>
#include "Common/Qt/AutoHeightTable.h"
#include "CommonFramework/Options/ConfigOption.h"

class QLineEdit;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


class TradeCountTableOption : public ConfigOption{
public:
    TradeCountTableOption();

    const std::vector<std::pair<std::string, int>>& list() const { return m_list; }

    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

    virtual void restore_defaults() override;

    virtual ConfigWidget* make_ui(QWidget& parent) override;

private:
    friend class TradeCountTableWidget;

    QString m_label;
    std::vector<std::pair<std::string, int>> m_list;
};


class TradeCountTableWidget : public QWidget, public ConfigWidget{
public:
    TradeCountTableWidget(QWidget& parent, TradeCountTableOption& value);

    virtual void restore_defaults() override;
    virtual void update_ui() override;

private:
    QLineEdit* make_count_box(QWidget& parent, int row, int count);

private:
    TradeCountTableOption& m_value;
    AutoHeightTableWidget* m_table;
    std::vector<QLineEdit*> m_entries;
};




}
}
}
#endif
