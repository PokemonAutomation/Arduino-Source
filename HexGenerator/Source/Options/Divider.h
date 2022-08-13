/*  Divider
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *      This isn't a real option. It's just a line divider between groups of options.
 */

#ifndef PokemonAutomation_Divider_H
#define PokemonAutomation_Divider_H

#include "ConfigItem.h"

namespace PokemonAutomation{
namespace HexGenerator{


class Divider : public ConfigItem{
public:
    static const std::string OPTION_TYPE;
    static const std::string JSON_TEXT;

public:
    Divider(const JsonObject& obj);

    virtual const std::string& type() const override{ return OPTION_TYPE; }
    virtual QWidget* make_ui(QWidget& parent) override;

private:
    friend class BooleanCheckBoxUI;
};



class DividerUI : public QWidget{
public:
    DividerUI(QWidget& parent, const std::string& label);
    ~DividerUI();
};


}
}
#endif
