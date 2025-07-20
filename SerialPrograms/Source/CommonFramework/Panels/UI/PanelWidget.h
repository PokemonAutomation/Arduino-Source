/*  Panel
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PanelWidget_H
#define PokemonAutomation_PanelWidget_H

#include <QWidget>
#include "CommonFramework/Panels/PanelTools.h"

namespace PokemonAutomation{

class CollapsibleGroupBox;

// A base class to define the UI widgets of a program panel.
// A PanelInstance can call make_widget() to create it.
// Its derived classes can call make_header() to create a collabspile program header
// that shows program title, link to online documentation and others.
class PanelWidget : public QWidget{
public:
    PanelWidget(
        QWidget& parent,
        PanelInstance& instance,
        PanelHolder& holder
    );
    virtual ~PanelWidget() = default;

    // return the panel instance
    PanelInstance& instance(){ return m_instance; }

protected:
    //  Generate a collapsible UI element that shows the program panel header.
    //  It contains the name of the program and its description.
    virtual CollapsibleGroupBox* make_header(QWidget& parent);

protected:
    PanelInstance& m_instance;
    PanelHolder& m_holder;
};




}
#endif
