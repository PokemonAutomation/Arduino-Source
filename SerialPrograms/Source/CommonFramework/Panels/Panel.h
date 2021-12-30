/*  Panel
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Panel_H
#define PokemonAutomation_Panel_H

#include <memory>
#include "Common/Compiler.h"
#include "CommonFramework/Logging/Logger.h"

class QJsonValue;
class QWidget;

namespace PokemonAutomation{

class PanelInstance;


struct PanelListener{
    virtual void on_panel_construct(std::unique_ptr<PanelInstance> panel) = 0;
    virtual Logger& raw_logger() = 0;
    virtual void on_busy(PanelInstance& panel) = 0;
    virtual void on_idle(PanelInstance& panel) = 0;
};



class PanelDescriptor{
public:
    PanelDescriptor(
        Color color,
        std::string identifier,
        QString category, QString display_name,
        QString doc_link,
        QString description
    );
    virtual ~PanelDescriptor() = default;

    Color color() const{ return m_color; }
    const std::string& identifier() const{ return m_identifier; }
    const QString& category() const{ return m_category; }
    const QString& display_name() const{ return m_display_name; }
    const QString& doc_link() const{ return m_doc_link; }
    const QString& description() const{ return m_description; }

    virtual std::unique_ptr<PanelInstance> make_panel() const = 0;

private:
    const Color m_color;
    const std::string m_identifier;
    const QString m_category;
    const QString m_display_name;
    const QString m_doc_link;
    const QString m_description;
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
    PanelInstance(const PanelDescriptor& descriptor);
    virtual ~PanelInstance() = default;

    const PanelDescriptor& descriptor() const{ return m_descriptor; }

    void save_settings() const;
    virtual QWidget* make_widget(QWidget& parent, PanelListener& listener);

public:
    //  Serialization
    void from_json();
    virtual void from_json(const QJsonValue& json){}
    virtual QJsonValue to_json() const;

protected:
    const PanelDescriptor& m_descriptor;
};





}
#endif
