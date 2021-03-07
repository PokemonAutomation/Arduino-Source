/*  Multi-Host Table
 *
 *  From: https://github.com/Mysticial/Pokemon-Automation-SwSh-Arduino-Scripts
 *
 */

#ifndef PokemonAutomation_MultiHostTable_H
#define PokemonAutomation_MultiHostTable_H

#include <vector>
#include <QTableWidget>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include "Common/Qt/Options/MultiHostTableOption.h"
#include "SingleStatementOption.h"

namespace PokemonAutomation{


class MultiHostTable : public SingleStatementOption, public MultiHostTableOption{
public:
    static const QString OPTION_TYPE;

public:
    MultiHostTable(const QJsonObject& obj);

    virtual const QString& type() const override{ return OPTION_TYPE; }
    virtual bool is_valid() const override;
    virtual void restore_defaults() override;

    virtual QJsonObject to_json() const override;
    virtual std::string to_cpp() const override;

    virtual QWidget* make_ui(QWidget& parent) override;

private:
    friend class MultiHostTableUI;
};


class MultiHostTableUI : public MultiHostTableOptionUI{
public:
    MultiHostTableUI(QWidget& parent, MultiHostTable& value);
};


}
#endif
