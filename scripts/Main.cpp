#include "./headers/Window.hpp"
#include <iostream>
#include <system_error>

int main(int argc, char** argv){
    Window window;
    if(!window.createWindow()){
        std::cerr << "Something went wrong while making the window\n";
    }

    while(window.updateWindow()){
        std::cout << "updating\n";
    }

    window.destroyWindow();
}