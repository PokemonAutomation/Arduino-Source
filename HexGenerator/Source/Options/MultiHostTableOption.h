/*  Multi-Host Table
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_MultiHostTable_H
#define PokemonAutomation_MultiHostTable_H

#include <vector>
#include <QTableWidget>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include "Common/Qt/Options/MultiHostTableBaseOption.h"
#include "SingleStatementOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class MultiHostTable : public SingleStatementOption{
public:
    static const std::string OPTION_TYPE;

public:
    MultiHostTable(const JsonObject& obj);

    virtual const std::string& type() const override{ return OPTION_TYPE; }
    virtual QString check_validity() const override;
    virtual void restore_defaults() override;

    virtual JsonObject to_json() const override;
    virtual std::string to_cpp() const override;

    virtual QWidget* make_ui(QWidget& parent) override;

private:
    friend class MultiHostTableUI;
    MultiHostSlotOptionFactory m_factory;
    EditableTableBaseOption m_table;
};



}
}
}
#endif
