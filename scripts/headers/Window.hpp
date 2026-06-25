#pragma once

#include <GLFW/glfw3.h>
#include <memory>

class Window {
    public:
        int createWindow();
        int updateWindow();
        int destroyWindow();


    private:
        GLFWwindow* window;
};