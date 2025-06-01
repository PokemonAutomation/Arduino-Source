/*  ML Label Images
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ML_LabelImages_H
#define PokemonAutomation_ML_LabelImages_H

#include "CommonFramework/Panels/PanelInstance.h"
#include "CommonFramework/Panels/UI/PanelWidget.h"


namespace PokemonAutomation{
namespace ML{



class LabelImages_Descriptor : public PanelDescriptor{
public:
    LabelImages_Descriptor();
};



class LabelImages : public PanelInstance{
public:
    LabelImages(const LabelImages_Descriptor& descriptor);
    virtual QWidget* make_widget(QWidget& parent, PanelHolder& holder) override;

public:
    //  Serialization
    virtual void from_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

private:
    friend class LabelImages_Widget;
};



class LabelImages_Widget : public PanelWidget{
public:
    static LabelImages_Widget* make(
        QWidget& parent,
        LabelImages& instance,
        PanelHolder& holder
    );

private:
    ~LabelImages_Widget();
    LabelImages_Widget(
        QWidget& parent,
        LabelImages& instance,
        PanelHolder& holder
    );
    void construct();

private:
};




}
}
#endif

