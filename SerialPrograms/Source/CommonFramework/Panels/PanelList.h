/*  Panel List
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PanelList_H
#define PokemonAutomation_PanelList_H

#include <memory>
#include <QListWidget>
#include "CommonFramework/Panels/PanelDescriptor.h"
#include "CommonFramework/Panels/PanelTools.h"

namespace PokemonAutomation{



class PanelList : public QListWidget{
public:
    static const std::string JSON_PROGRAM_PANEL;

public:
    PanelList(QTabWidget& parent, PanelHolder& holder, std::string label, std::vector<PanelEntry> list);

    const std::string& label() const{ return m_label; }
    size_t items() const{ return m_panel_map.size(); }

    void set_panel(const std::string& panel_name);

protected:
    void handle_panel_clicked(const std::string& text);

protected:
    std::string m_label;
    PanelHolder& m_panel_holder;
private:
    std::map<std::string, std::unique_ptr<PanelDescriptor>> m_panel_map;
};




}
#endif
