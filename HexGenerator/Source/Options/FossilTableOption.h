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
#include "Common/PokemonSwSh/PokemonSwSh_FossilTable.h"
#include "SingleStatementOption.h"

namespace PokemonAutomation{
namespace HexGenerator{


#if 1
class FossilTable : public SingleStatementOption{
public:
    static const std::string OPTION_TYPE;

    static const std::vector<std::string> FOSSIL_LIST;

public:
    FossilTable(const JsonObject& obj);

    virtual const std::string& type() const override{ return OPTION_TYPE; }
    virtual std::string check_validity() const override;
    virtual void restore_defaults() override;

    virtual JsonObject to_json() const override;
    virtual std::string to_cpp() const override;

    virtual QWidget* make_ui(QWidget& parent) override;

private:
    friend class FossilTableUI;
    NintendoSwitch::PokemonSwSh::FossilTable m_table;
};
#endif




}
}
#endif
