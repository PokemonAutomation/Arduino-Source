/*  Batch Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Options_BatchWidget_H
#define PokemonAutomation_Options_BatchWidget_H

#include <QWidget>
#include "Common/Cpp/Options/BatchOption.h"
#include "ConfigWidget.h"

namespace PokemonAutomation{



class BatchWidget : public QWidget, public ConfigWidget{
public:
    ~BatchWidget();
    BatchWidget(QWidget& parent, BatchOption& value);

    virtual void update_value() override;
    virtual void value_changed(void* object) override;

protected:
    BatchOption& m_value;
    std::vector<ConfigWidget*> m_options;
};




}
#endif
