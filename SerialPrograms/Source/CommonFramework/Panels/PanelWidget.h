/*  Panel
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PanelWidget_H
#define PokemonAutomation_PanelWidget_H

#include <QWidget>
#include "Panel.h"

namespace PokemonAutomation{


class PanelWidget : public QWidget{
public:
    PanelWidget(
        QWidget& parent,
        PanelInstance& instance,
        PanelListener& listener
    );
    virtual ~PanelWidget() = default;

    PanelInstance& instance(){ return m_instance; }

protected:
    virtual QWidget* make_header(QWidget& parent);

protected:
    PanelInstance& m_instance;
    PanelListener& m_listener;
};




}
#endif
