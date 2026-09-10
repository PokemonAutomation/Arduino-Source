/*  Panel Session
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PanelSession_H
#define PokemonAutomation_PanelSession_H

#include "Common/Cpp/UiWrapper.h"
#include "PanelDescriptor.h"

class QWidget;

namespace PokemonAutomation{

class JsonValue;

// Class to represent one instance of a pokemon automation program.
// Since programs are listed in the program panels, so this class is called PanelSession.
// Its derived classes hold all the program data and program logic. It also calls
// `make_widget()` to generate the UI for the program.
class PanelSession : public UiState<PanelSession>{
public:
    explicit PanelSession(const PanelDescriptor& descriptor);
    virtual ~PanelSession() = default;

    const PanelDescriptor& descriptor() const{ return m_descriptor; }

    void save_settings() const;
    void validate_resource_list();


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
