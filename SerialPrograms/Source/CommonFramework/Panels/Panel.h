/*  Panel
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Panel_H
#define PokemonAutomation_Panel_H

#include <memory>
#include <QString>
#include "Common/Compiler.h"
#include "CommonFramework/Logging/LoggerQt.h"

class QWidget;

namespace PokemonAutomation{

class JsonValue;
class PanelInstance;
class PanelDescriptor;


struct PanelHolder{
    //  Returns true if ready for new panel.
    virtual bool report_new_panel_intent(const PanelDescriptor& descriptor) = 0;

    virtual void load_panel(std::unique_ptr<PanelInstance> panel) = 0;
    virtual LoggerQt& raw_logger() = 0;
    virtual void on_busy(PanelInstance& panel) = 0;
    virtual void on_idle(PanelInstance& panel) = 0;
};



class PanelDescriptor{
public:
    PanelDescriptor(
        Color color,
        std::string identifier,
        std::string category, std::string display_name,
        std::string doc_link,
        std::string description
    );
    virtual ~PanelDescriptor() = default;

    Color color() const{ return m_color; }
    const std::string& identifier() const{ return m_identifier; }
    const std::string& category() const{ return m_category; }
    const std::string& display_name() const{ return m_display_name; }
    const std::string& doc_link() const{ return m_doc_link; }
    const std::string& description() const{ return m_description; }

    virtual std::unique_ptr<PanelInstance> make_panel() const = 0;

private:
    const Color m_color;
    const std::string m_identifier;
    const std::string m_category;
    const std::string m_display_name;
    const std::string m_doc_link;
    const std::string m_description;
};

template <typename Descriptor, typename Instance>
class PanelDescriptorWrapper : public Descriptor{
public:
    using Descriptor::Descriptor;
    virtual std::unique_ptr<PanelInstance> make_panel() const override{
        return std::unique_ptr<PanelInstance>(new Instance(*this));
    }
};



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
