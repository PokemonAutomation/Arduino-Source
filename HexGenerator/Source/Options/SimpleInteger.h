/*  Simple Integer
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_SimpleInteger_H
#define PokemonAutomation_SimpleInteger_H

#include "Common/Qt/Options/SimpleIntegerOption.h"
#include "SingleStatementOption.h"

namespace PokemonAutomation{
namespace HexGenerator{


class SimpleInteger : public SingleStatementOption{
public:
    static const std::string OPTION_TYPE;
    static const std::string JSON_MIN_VALUE;
    static const std::string JSON_MAX_VALUE;

public:
    SimpleInteger(const JsonObject& obj);

    virtual const std::string& type() const override{ return OPTION_TYPE; }
    virtual std::string check_validity() const override;
    virtual void restore_defaults() override;

    virtual JsonObject to_json() const override;
    virtual std::string to_cpp() const override;

    virtual QWidget* make_ui(QWidget& parent) override;

private:
    SimpleIntegerOption<uint32_t> m_option;
};



}
}
#endif


