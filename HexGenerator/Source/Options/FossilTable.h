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
#include "Common/Qt/Options/FossilTableOption.h"
#include "SingleStatementOption.h"

namespace PokemonAutomation{


class FossilTable : public SingleStatementOption, public FossilTableOption{
public:
    static const QString OPTION_TYPE;

public:
    FossilTable(const QJsonObject& obj);

    virtual const QString& type() const override{ return OPTION_TYPE; }
    virtual bool is_valid() const override;
    virtual void restore_defaults() override;

    virtual QJsonObject to_json() const override;
    virtual std::string to_cpp() const override;

    virtual QWidget* make_ui(QWidget& parent) override;

private:
    friend class FossilTableUI;
};


class FossilTableUI : public FossilTableOptionUI{
public:
    FossilTableUI(QWidget& parent, FossilTable& value);
};


}
#endif
