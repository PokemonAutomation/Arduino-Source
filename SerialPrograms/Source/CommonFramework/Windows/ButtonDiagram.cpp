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

// #include <iostream>
// using std::cout;
// using std::endl;


namespace PokemonAutomation{

const char* NS1_PRO_CONTROLLER_MAPPING_PATH = "/NintendoSwitch/Layout-ProController.png";
const char* NS1_JOYCON_VERTICAL_MAPPING_PATH = "/NintendoSwitch/Layout-JoyconVertical.png";
const char* NS1_JOYCON_HORIZONTAL_MAPPING_PATH = "/NintendoSwitch/Layout-JoyconHorizontal.png";

const char* NS2_PRO_CONTROLLER_MAPPING_PATH = "/NintendoSwitch2/Layout-ProController.png";


ButtonDiagram::ButtonDiagram(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("Controller Keyboard Mapping");

    QMenuBar* menu = menuBar();
    QMenu* ns1_pro_controller = menu->addMenu("NS1: Pro Controller");
    QMenu* ns1_joycon_vertical = menu->addMenu("NS1: Joycon (Vertical)");
    QMenu* ns1_joycon_horizontal = menu->addMenu("NS1: Joycon (Horizontal)");
    QMenu* ns2_pro_controller = menu->addMenu("NS2: Pro Controller");

//    pro_controller->addAction("asdfadf");

    connect(
        ns1_pro_controller, &QMenu::aboutToShow,
        this, [this](){
            set_image(NS1_PRO_CONTROLLER_MAPPING_PATH);
        }
    );
    connect(
        ns1_joycon_vertical, &QMenu::aboutToShow,
        this, [this](){
            set_image(NS1_JOYCON_VERTICAL_MAPPING_PATH);
        }
    );
    connect(
        ns1_joycon_horizontal, &QMenu::aboutToShow,
        this, [this](){
            set_image(NS1_JOYCON_HORIZONTAL_MAPPING_PATH);
        }
    );
    connect(
        ns2_pro_controller, &QMenu::aboutToShow,
        this, [this](){
            set_image(NS2_PRO_CONTROLLER_MAPPING_PATH);
        }
    );

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

    set_image(NS2_PRO_CONTROLLER_MAPPING_PATH);

    add_window(*this);
}
ButtonDiagram::~ButtonDiagram(){
    remove_window(*this);
}

void ButtonDiagram::set_image(const std::string& resource_name){
    const std::string image_path = RESOURCE_PATH() + resource_name;
    m_image = QPixmap(QString::fromStdString(image_path));
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

