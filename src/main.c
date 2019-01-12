#include <stdio.h>

#include "game.h"

int main(int argc, char **argv)
{
    if (!glfwInit())
    {
        printf("Couldn't initialize GLFW.\n");
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);

    GLFWwindow *window = glfwCreateWindow(1280, 720, "CCraft 0.0.0", NULL, NULL);
    if (!window)
    {
        printf("Couldn't create the window.\n");
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    if(!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress))
    {
        printf("Couldn't initialize GLAD.\n");
        return -1;
    }

    game g;
    input i;

    glfwSetWindowUserPointer(window, &i);

    game_init(&g, window);
    input_init(&i, window);

    double last_time;
    double current_time;
    
    while (!glfwWindowShouldClose(window))
    {
        current_time = glfwGetTime();

        game_handle_input(&g, &i);
        game_update(&g, current_time - last_time);
        game_draw(&g);

        input_end_frame(&i);

        glfwSwapBuffers(window);
        glfwPollEvents();

        last_time = current_time;
    }

    game_destroy(&g);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}