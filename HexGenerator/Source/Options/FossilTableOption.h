/*  Fossil Table
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_FossilTable_H
#define PokemonAutomation_FossilTable_H

#include <vector>
#include <QTableWidget>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include "Common/Qt/Options/FossilTableBaseOption.h"
#include "SingleStatementOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


#if 1
class FossilTable : public SingleStatementOption{
public:
    static const std::string OPTION_TYPE;

public:
    FossilTable(const JsonObject& obj);

    virtual const std::string& type() const override{ return OPTION_TYPE; }
    virtual QString check_validity() const override;
    virtual void restore_defaults() override;

    virtual JsonObject to_json() const override;
    virtual std::string to_cpp() const override;

    virtual QWidget* make_ui(QWidget& parent) override;

private:
    friend class FossilTableUI;
    FossilGameOptionFactory m_factory;
    EditableTableBaseOption m_table;
};
#endif




}
}
}
#endif
