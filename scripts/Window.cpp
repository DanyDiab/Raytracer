#include <GLFW/glfw3.h>
#include <memory>
#include "./headers/Window.hpp"

int Window::createWindow(){

    if (!glfwInit()) return -1;

    window = glfwCreateWindow(960, 540, "RayTracer", NULL, NULL);

    if (!window){
        glfwTerminate();
        return -1;
    }


    glfwMakeContextCurrent(window);

    while (!glfwWindowShouldClose(window))
    {

    }

    glfwTerminate();
    return 0;
}


int Window::updateWindow(){
    int status = glfwWindowShouldClose(window);

    glfwSwapBuffers(window);

    glfwPollEvents();
    return status;
}


int Window::destroyWindow(){
    glfwDestroyWindow(window);
    return 0;
}