/*  Button Diagram
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QLabel>
#include <QMainWindow>

namespace PokemonAutomation{

class ButtonDiagram : public QMainWindow{
public:
    ButtonDiagram(QWidget& parent);

private:
    void resizeEvent(QResizeEvent*);

private:
    QPixmap m_image;
    QLabel* m_image_label;
};


}
