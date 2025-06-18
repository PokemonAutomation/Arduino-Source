/*  Panel Instance
 *
 *  From: https://github.com/PokemonAutomation/
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

// Class to represent one instance of a pokemon automation program.
// Since programs are listed in the program panels, so this class is called PanelInstance.
// Its derived classes hold all the program data and program logic. It also calls
// `make_widget()` to generate the UI for the program.
class PanelInstance{
public:
    explicit PanelInstance(const PanelDescriptor& descriptor);
    virtual ~PanelInstance() = default;

    const PanelDescriptor& descriptor() const{ return m_descriptor; }

    void save_settings() const;

public:
    // The implmentation is defined in "UI/PanelWidget.h" to avoid circular dependency
    // Returns a UI/PanelWidget.h:PanelWidget
    virtual QWidget* make_widget(QWidget& parent, PanelHolder& holder);

public:
    //  Serialization
    void from_json();
    virtual void from_json([[maybe_unused]] const JsonValue& json){}
    virtual JsonValue to_json() const;

protected:
    const PanelDescriptor& m_descriptor;
};



}
#endif
