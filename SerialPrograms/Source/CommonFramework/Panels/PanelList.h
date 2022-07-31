/*  Panel List
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PanelList_H
#define PokemonAutomation_PanelList_H

#include <memory>
#include <QListWidget>
#include "CommonFramework/Panels/Panel.h"

namespace PokemonAutomation{


class PanelList : public QListWidget{
public:
    static const std::string JSON_PROGRAM_PANEL;

public:
    PanelList(QTabWidget& parent, std::string label, PanelHolder& holder);

    const std::string& label() const{ return m_label; }
    size_t items() const{ return m_panels.size(); }

    void set_panel(const std::string& panel_name);

protected:
    void add_divider(std::string label);

    void handle_panel_clicked(const std::string& text);

    template <typename Descriptor, typename Instance>
    void add_settings(){
        add_program<Descriptor, Instance>();
        //  Need to force initialize a settings panel so that it loads and
        //  updates the globals from serialization.
        m_panels.back().second->make_panel()->from_json();
    }

    template <typename Descriptor, typename Instance>
    void add_program(){
        std::unique_ptr<PanelDescriptor> panel(
            new PanelDescriptorWrapper<Descriptor, Instance>()
        );
        m_panels.emplace_back(panel->display_name(), std::move(panel));
    }

    void add_panel(std::unique_ptr<PanelDescriptor> panel){
        const std::string& name = panel->display_name();
        m_panels.emplace_back(name, std::move(panel));
    }

    void finish_panel_setup();

protected:
    std::string m_label;
    PanelHolder& m_panel_holder;
    std::vector<std::pair<std::string, std::unique_ptr<PanelDescriptor>>> m_panels;
private:
    std::map<std::string, const PanelDescriptor*> m_panel_map;
};




}
#endif
