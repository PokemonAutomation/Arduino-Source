/*  WindowTracker.cpp
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <set>
#include <mutex>
#include <QMainWindow>
#include "WindowTracker.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{



std::mutex window_lock;
std::set<QMainWindow*> open_windows;

void add_window(QMainWindow& window){
    std::lock_guard<std::mutex> lock(window_lock);
    open_windows.insert(&window);
}
void remove_window(QMainWindow& window){
    std::lock_guard<std::mutex> lock(window_lock);
    open_windows.erase(&window);
}
void close_all_windows(){
    while (true){
        QMainWindow* window = nullptr;
        {
            std::lock_guard<std::mutex> lock(window_lock);
//            cout << "open_windows.size() = " << open_windows.size() << endl;
            if (open_windows.empty()){
                return;
            }
            auto iter = open_windows.begin();
            window = *iter;
            open_windows.erase(iter);
        }
        window->close();
    }
}




}
