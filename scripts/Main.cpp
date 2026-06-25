#include "./headers/Window.hpp"
#include <iostream>

int main(int argc, char** argv){
    Window window;
    window.createWindow();

    while(window.updateWindow()){
        std::cout << "updating\n";
    }

    window.destroyWindow();
}