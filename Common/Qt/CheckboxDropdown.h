/*  Command Row
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_CheckboxDropdown_H
#define PokemonAutomation_CheckboxDropdown_H

#include <QEvent>
#include <QAbstractItemView>
#include <QStandardItemModel>
#include <QComboBox>

namespace PokemonAutomation{

class CheckboxDropdownItem : public QObject, public QStandardItem{
    Q_OBJECT
public:
    using QStandardItem::QStandardItem;

    void setChecked(bool checked){
        if (checked){
            this->setCheckState(Qt::Checked);
        }else{
            this->setCheckState(Qt::Unchecked);
        }
    }
    bool checked() const{
        return this->checkState() == Qt::Checked;
    }

signals:
    void checkStateChanged(Qt::CheckState state);
};




class CheckboxDropdown : public QComboBox{
    Q_OBJECT
public:
    CheckboxDropdown(QWidget* parent, const QString& label)
        : QComboBox(parent)
    {
        m_model = new QStandardItemModel(this);
        this->setModel(m_model);

        QComboBox::addItem(label);

        this->view()->viewport()->installEventFilter(this);
    }

    CheckboxDropdownItem* addItem(const QString& text, Qt::CheckState = Qt::Unchecked){
        CheckboxDropdownItem* item = new CheckboxDropdownItem(text);
        item->setCheckable(true);
        item->setCheckState(Qt::Unchecked);
        m_model->appendRow(item);
        m_items.emplace_back(item);
        return item;
    }


public:
    size_t size() const{
        return m_items.size();
    }
    CheckboxDropdownItem* operator[](size_t index) const{
        return m_items[index];
    }

    void setLabel(const QString& label){
        setItemText(0, label);
    }

    virtual void wheelEvent(QWheelEvent* event) override{
        QWidget::wheelEvent(event);
    }
    virtual bool eventFilter(QObject* obj, QEvent* event) override{
//        cout << "eventFilter()" << endl;
        if (event->type() != QEvent::MouseButtonRelease){
            return QComboBox::eventFilter(obj, event);
        }
        int index = view()->currentIndex().row();
        if (index <= 0){
            return QComboBox::eventFilter(obj, event);
        }
        if (itemData(index, Qt::CheckStateRole) == Qt::Checked){
            setItemData(index, Qt::Unchecked, Qt::CheckStateRole);
            emit m_items[index - 1]->checkStateChanged(Qt::Unchecked);
        }else{
            setItemData(index, Qt::Checked, Qt::CheckStateRole);
            emit m_items[index - 1]->checkStateChanged(Qt::Checked);
        }
        return true;
    }

private:
    QStandardItemModel* m_model;
    std::vector<CheckboxDropdownItem*> m_items;
};


}
#endif
