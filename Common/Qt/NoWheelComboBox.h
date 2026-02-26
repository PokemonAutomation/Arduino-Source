/*  ComboBox without mouse wheel scrolling. Also, the height has been set to be more compact.
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NoWheelComboBox_H
#define PokemonAutomation_NoWheelComboBox_H

#include <QComboBox>
#include <QAbstractItemView>
#include <QStyledItemDelegate>

//#define PA_ENABLE_SIZE_CACHING

namespace PokemonAutomation{


class HeightDelegate : public QStyledItemDelegate {
public:
    using QStyledItemDelegate::QStyledItemDelegate;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        QSize s = QStyledItemDelegate::sizeHint(option, index);
        int vertical_padding = 8; 
        s.setHeight(option.fontMetrics.height() + vertical_padding);
        return s;
    }
};


class NoWheelCompactComboBox : public QComboBox{
public:
    explicit NoWheelCompactComboBox(QWidget* parent = nullptr) : QComboBox(parent) {
        // Set the height for every line in the dropdown
        this->view()->setItemDelegate(new HeightDelegate(this));

        // this->setStyleSheet("QAbstractItemView::item { height: 100px; }");
        
        // Optional: Force a standard list view to ensure the stylesheet 
        // is respected on all platforms (like Windows/macOS)
        // #include <QListView>
        // this->setView(new QListView()); 
    }

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
