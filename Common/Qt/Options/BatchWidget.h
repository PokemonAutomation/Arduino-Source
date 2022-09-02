/*  Batch Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Options_BatchWidget_H
#define PokemonAutomation_Options_BatchWidget_H

#include <QWidget>
#include "Common/Cpp/Options/BatchOption.h"
#include "ConfigWidget.h"

namespace PokemonAutomation{



class BatchWidget : public QWidget, public ConfigWidget, private ConfigOption::Listener{
public:
    ~BatchWidget();
    BatchWidget(QWidget& parent, BatchOption& value);

    virtual void update() override;
    virtual void value_changed() override;

protected:
    BatchOption& m_value;
    std::vector<ConfigWidget*> m_options;
};




}
#endif
