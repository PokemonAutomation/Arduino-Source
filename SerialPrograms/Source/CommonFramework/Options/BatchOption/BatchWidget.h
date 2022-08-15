/*  Batch Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_BatchWidget_H
#define PokemonAutomation_BatchWidget_H

#include <QWidget>
#include "Common/Qt/Options/ConfigWidget.h"
#include "BatchOption.h"

namespace PokemonAutomation{



class BatchWidget : public QWidget, public ConfigWidget{
public:
    BatchWidget(QWidget& parent, BatchOption& value);

    virtual void update() override;

protected:
    BatchOption& m_value;
    std::vector<ConfigWidget*> m_options;
};




}
#endif
