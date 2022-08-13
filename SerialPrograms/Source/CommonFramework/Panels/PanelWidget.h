/*  Panel
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PanelWidget_H
#define PokemonAutomation_PanelWidget_H

#include <QWidget>
#include "PanelTools.h"

namespace PokemonAutomation{

class CollapsibleGroupBox;

class PanelWidget : public QWidget{
public:
    PanelWidget(
        QWidget& parent,
        PanelInstance& instance,
        PanelHolder& holder
    );
    virtual ~PanelWidget() = default;

    PanelInstance& instance(){ return m_instance; }

protected:
    virtual CollapsibleGroupBox* make_header(QWidget& parent);

protected:
    PanelInstance& m_instance;
    PanelHolder& m_holder;
};




}
#endif
