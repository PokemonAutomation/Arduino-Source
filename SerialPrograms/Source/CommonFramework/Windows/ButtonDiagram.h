/*  Button Diagram
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QLabel>
#include <QMainWindow>

namespace PokemonAutomation{

class ButtonDiagram : public QMainWindow{
public:
    ButtonDiagram(QWidget* parent = nullptr);
    ~ButtonDiagram();

private:
    void set_image(const std::string& resource_name);
    void resizeEvent(QResizeEvent*);

private:
    QPixmap m_image;
    QLabel* m_image_label;
};


}
