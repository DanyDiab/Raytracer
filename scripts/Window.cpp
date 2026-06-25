#include <GLFW/glfw3.h>

int main(int argc, char** argv){

    GLFWwindow* window;

    if (!glfwInit())
        return -1;

    window = glfwCreateWindow(960, 540, "RayTracer", NULL, NULL);

     if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    while (!glfwWindowShouldClose(window))
    {
        glfwSwapBuffers(window);

        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}