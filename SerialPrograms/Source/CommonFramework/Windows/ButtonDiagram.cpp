/*  Button Diagram
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QCoreApplication>
#include <QMessageBox>
#include <QMenuBar>
#include "CommonFramework/Globals.h"
#include "WindowTracker.h"
#include "ButtonDiagram.h"

//#include <iostream>
//using std::cout;
//using std::endl;


namespace PokemonAutomation{


ButtonDiagram::ButtonDiagram(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("Controller Keyboard Mapping");

    QMenuBar* menu = menuBar();
    QMenu* pro_controller = menu->addMenu("Pro Controller");
    QMenu* joycon_vertical = menu->addMenu("Joycon (Vertical)");
    QMenu* joycon_horizontal = menu->addMenu("Joycon (Horizontal)");

//    pro_controller->addAction("asdfadf");

    connect(
        pro_controller, &QMenu::aboutToShow,
        this, [this](){
            set_image("/NintendoSwitch/Layout-ProController.png");
        }
    );
    connect(
        joycon_vertical, &QMenu::aboutToShow,
        this, [this](){
            set_image("/NintendoSwitch/Layout-JoyconVertical.png");
        }
    );
    connect(
        joycon_horizontal, &QMenu::aboutToShow,
        this, [this](){
            set_image("/NintendoSwitch/Layout-JoyconHorizontal.png");
        }
    );


//    m_image = QPixmap(QString::fromStdString(RESOURCE_PATH() + "/NintendoSwitch/Layout-ProController.jpg"));

    m_image_label = new QLabel(this);
    setCentralWidget(m_image_label);
    m_image_label->setAlignment(Qt::AlignCenter);
//    m_image_label->setPixmap(m_image.scaled(800, 600, Qt::KeepAspectRatio));
//    m_image_label->setPixmap(m_image);
//    m_image_label->setScaledContents(true);
    m_image_label->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

//    image_label->resize(800, 600);
//    image_label->setFixedSize(800, 600);

    resize(800, 600 + menu->sizeHint().height());

    set_image("/NintendoSwitch/Layout-ProController.jpg");

    add_window(*this);
}
ButtonDiagram::~ButtonDiagram(){
    remove_window(*this);
}

void ButtonDiagram::set_image(const std::string& resource_name){
    m_image = QPixmap(QString::fromStdString(RESOURCE_PATH() + resource_name));
    ButtonDiagram::resizeEvent(nullptr);
}
void ButtonDiagram::resizeEvent(QResizeEvent*){
    int iw = m_image.width();
    int ih = m_image.height();
    int ww = m_image_label->width();
    int wh = m_image_label->height();
//    cout << "ww = " << ww << ", wh = " << wh << endl;

    double scale_w = (double)ww / iw;
    double scale_h = (double)wh / ih;
    double scale = std::min(scale_w, scale_h);

    iw = (int)(iw * scale);
    ih = (int)(ih * scale);

    m_image_label->setPixmap(m_image.scaled(iw, ih, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}



}

