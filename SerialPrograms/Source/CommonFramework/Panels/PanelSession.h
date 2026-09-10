/*  Panel Session
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PanelSession_H
#define PokemonAutomation_PanelSession_H

#include "PanelDescriptor.h"

class QWidget;

namespace PokemonAutomation{

class JsonValue;
struct PanelHolder;

// Class to represent one instance of a pokemon automation program.
// Since programs are listed in the program panels, so this class is called PanelSession.
// Its derived classes hold all the program data and program logic. It also calls
// `make_widget()` to generate the UI for the program.
class PanelSession{
public:
    explicit PanelSession(const PanelDescriptor& descriptor);
    virtual ~PanelSession() = default;

    const PanelDescriptor& descriptor() const{ return m_descriptor; }

    void save_settings() const;
    void validate_resource_list();


public:
    // The implmentation is defined in "UI/PanelWidget.h" to avoid circular dependency
    // Returns a UI/PanelWidget.h:PanelWidget
    virtual QWidget* make_widget(QWidget& parent);


public:
    //  Serialization
    void load_json();
    virtual JsonValue to_json() const;
    virtual void load_json(const JsonValue& json);


protected:
    const PanelDescriptor& m_descriptor;
};



}
#endif
