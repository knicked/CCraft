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

    double last_time = 0.0;
    double current_time = 0.0;
    double delta_time = 0.0;
    double tick_accumulator = 0.0;
    const double tick_interval = 1.0 / 20.0;

    while (!glfwWindowShouldClose(window))
    {
        current_time = glfwGetTime();
        delta_time = current_time - last_time;
        tick_accumulator += delta_time;

        while (tick_accumulator >= tick_interval)
        {
            game_handle_input(&g, &i);
            game_tick(&g);
            input_end_frame(&i);
            tick_accumulator -= tick_interval;
        }

        game_draw(&g);

        glfwSwapBuffers(window);
        glfwPollEvents();

        last_time = current_time;
    }

    game_destroy(&g);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}