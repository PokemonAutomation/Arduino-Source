/*  ComboBox without mouse wheel scrolling.
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NoWheelComboBox_H
#define PokemonAutomation_NoWheelComboBox_H

#include <QComboBox>

//#define PA_ENABLE_SIZE_CACHING

namespace PokemonAutomation{


class NoWheelComboBox : public QComboBox{
public:
    using QComboBox::QComboBox;

    void update_size_cache(){
#ifdef PA_ENABLE_SIZE_CACHING
        m_cached_size = QComboBox::sizeHint();
        m_cached_minimum_size = QComboBox::minimumSizeHint();
#endif
    }

    virtual void wheelEvent(QWheelEvent* event) override{
        QWidget::wheelEvent(event);
    }

#ifdef PA_ENABLE_SIZE_CACHING
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
#endif
};



}
#endif
