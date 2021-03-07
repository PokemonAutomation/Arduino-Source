/*  Button Diagram
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QCoreApplication>
//#include <QFile>
#include <QMessageBox>
//#include <QGraphicsScene>
//#include <QGraphicsView>
//#include <QGraphicsPixmapItem>
#include "ButtonDiagram.h"

#include <iostream>
using std::cout;
using std::endl;

ButtonDiagram::ButtonDiagram(QWidget& parent)
    : QMainWindow(&parent)
{
    setWindowTitle("Controller Keyboard Mapping");

    //  Find the image.
    QString path = QCoreApplication::applicationDirPath() + "/";
    for (int c = 0; c < 5; c++){
//        cout << (path + "Button Layout.jpg").toUtf8().data() << endl;
        QString filepath = path + "Button Layout.jpg";
//        QFile file(filepath);
//        QMessageBox box0;
//        box0.critical(nullptr, "Error", "Exists = " + QString::number(file.exists()));
        m_image = QPixmap(filepath);
        if (!m_image.isNull()){
            break;
        }
//        QMessageBox box;
//        box.critical(nullptr, "Error", path + "Button Layout.jpg");
        path += "../";
    }

    m_image_label = new QLabel(this);
    setCentralWidget(m_image_label);
    m_image_label->setAlignment(Qt::AlignCenter);
//    m_image_label->setPixmap(m_image.scaled(800, 600, Qt::KeepAspectRatio));
//    m_image_label->setPixmap(m_image);
//    m_image_label->setScaledContents(true);
    m_image_label->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

//    image_label->resize(800, 600);
//    image_label->setFixedSize(800, 600);

    resize(800, 600);
}

void ButtonDiagram::resizeEvent(QResizeEvent*){
//    cout << "asdf" << endl;
    int iw = m_image.width();
    int ih = m_image.height();
    int ww = m_image_label->width();
    int wh = m_image_label->height();

    double scale_w = (double)ww / iw;
    double scale_h = (double)wh / ih;
    double scale = std::min(scale_w, scale_h);

    iw = (int)(iw * scale);
    ih = (int)(ih * scale);

    m_image_label->setPixmap(m_image.scaled(iw, ih, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

