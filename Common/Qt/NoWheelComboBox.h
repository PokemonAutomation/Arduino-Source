/*  ComboBox without mouse wheel scrolling.
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_NoWheelComboBox_H
#define PokemonAutomation_NoWheelComboBox_H

#include <QComboBox>

namespace PokemonAutomation{


class NoWheelComboBox : public QComboBox{
public:
    using QComboBox::QComboBox;

    void update_size_cache(){
        m_cached_size = QComboBox::sizeHint();
        m_cached_minimum_size = QComboBox::minimumSizeHint();
    }

    virtual void wheelEvent(QWheelEvent*) override{}

    virtual QSize sizeHint() const override{
        if (m_cached_size.isValid()){
            return m_cached_size;
        }
        return QComboBox::sizeHint();
    }
    virtual QSize minimumSizeHint() const override{
        if (m_cached_minimum_size.isValid()){
            return m_cached_minimum_size;
        }
        return QComboBox::minimumSizeHint();
    }

protected:
    QSize m_cached_size;
    QSize m_cached_minimum_size;
};



}
#endif
