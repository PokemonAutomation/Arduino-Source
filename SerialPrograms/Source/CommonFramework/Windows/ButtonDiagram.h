/*  Button Diagram
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QLabel>
#include <QMainWindow>

namespace PokemonAutomation{

// The window that shows button mapping: which keyboard keys are mapped to which
// Switch controller buttons.
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
