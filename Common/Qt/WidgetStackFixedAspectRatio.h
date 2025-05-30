/*  Widget Stack Fixed Aspect Ratio
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_WidgetStackFixedAspectRatio_H
#define PokemonAutomation_WidgetStackFixedAspectRatio_H

#include <set>
#include <QWidget>
#include "Common/Cpp/ValueDebouncer.h"

namespace PokemonAutomation{


class WidgetStackFixedAspectRatio : public QWidget{
public:
    enum SizePolicy{
        EXPAND_TO_BOX,
        ADJUST_HEIGHT_TO_WIDTH,
    };

public:
    WidgetStackFixedAspectRatio(
        QWidget& parent,
        SizePolicy size_policy,
        double aspect_ratio = 16/9.
    );

    double sanitize_aspect_ratio(double aspect_ratio) const;

    void set_size_policy(SizePolicy size_policy);
    void set_aspect_ratio(double aspect_ratio);
    void set_all(SizePolicy size_policy, double aspect_ratio);

    void add_widget(QWidget& widget);
    void remove_widget(QWidget* widget);

    virtual void resizeEvent(QResizeEvent* event) override;

private:
    void clear_fixed_dimensions();
    void resize_to_box(QSize enclosing_box);
    void resize_to_width(int width);

    void update_size(QSize size);

private:
    SizePolicy m_size_policy;
    double m_aspect_ratio;
    std::set<QWidget*> m_widgets;

    QWidget* m_detached_internal;
    QWidget* m_stack_holder;

    ValueDebouncer<int> m_debouncer;
};



}
#endif
