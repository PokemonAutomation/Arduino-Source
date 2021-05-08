
#include <QApplication>
#include "Common/Qt/StringException.h"
#ifdef PA_TESSERACT
#include "Tesseract/capi.h"
#endif
#include "PersistentSettings.h"
#include "CrashDump.h"
#include "Windows/MainWindow.h"

#include <iostream>
using std::cout;
using std::endl;


using namespace PokemonAutomation;


int main(int argc, char *argv[]){
    setup_crash_handler();

    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication application(argc, argv);

#ifdef PA_TESSERACT
    cout << "Tesseract Version: " << TessVersion() << endl;
#endif

    try{
        settings.read();
    }catch (const StringException& error){
        cout << error.message().toUtf8().data() << endl;
    }
//    int* ptr = nullptr;
//    cout << *ptr << endl;

    MainWindow w;
    w.show();
    int ret = application.exec();
    settings.write();
    return ret;
}
