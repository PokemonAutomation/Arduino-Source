/*  Panel Instance
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PanelInstance_H
#define PokemonAutomation_PanelInstance_H

#include "Common/Compiler.h"
#include "PanelDescriptor.h"

class QWidget;

namespace PokemonAutomation{

class JsonValue;
struct PanelHolder;


class PanelInstance{
public:
    explicit PanelInstance(const PanelDescriptor& descriptor);
    virtual ~PanelInstance() = default;

    const PanelDescriptor& descriptor() const{ return m_descriptor; }

    void save_settings() const;
    virtual QWidget* make_widget(QWidget& parent, PanelHolder& holder);

public:
    //  Serialization
    void from_json();
    virtual void from_json(const JsonValue& json){}
    virtual JsonValue to_json() const;

protected:
    const PanelDescriptor& m_descriptor;
};



}
#endif
