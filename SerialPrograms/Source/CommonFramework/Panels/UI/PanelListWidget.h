/*  Panel List Widget
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PanelListWidget_H
#define PokemonAutomation_PanelListWidget_H

#include <QListWidget>
#include "CommonFramework/Panels/PanelList.h"

namespace PokemonAutomation{



class PanelListWidget : public QListWidget{
public:
    static const std::string JSON_PROGRAM_PANEL;

public:
    PanelListWidget(
        QWidget& parent, PanelHolder& holder,
        std::vector<PanelEntry> list
    );

    size_t items() const{ return m_panel_map.size(); }

    void set_panel(const std::string& panel_name);

private:
    void handle_panel_clicked(const std::string& text);

private:
    PanelHolder& m_panel_holder;
    std::map<std::string, std::shared_ptr<const PanelDescriptor>> m_panel_map;
};



}
#endif
