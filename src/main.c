#include "game.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define FPS 60.0

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

    GLFWwindow *window = glfwCreateWindow(1280, 720, "CCraft", NULL, NULL);
    if (!window)
    {
        printf("Couldn't create the window.\n");
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(0);

    if(!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress))
    {
        printf("Couldn't initialize GLAD.\n");
        return -1;
    }

    game g;
    input i;

    g.server_addr.sin_port = htons(25565);
    g.online = 0;
    strcpy(g.player_nickname, "Unnamed");

    glfwSetWindowUserPointer(window, &i);

    #ifdef _WIN32
        WSADATA wsa_data;
        WSAStartup(MAKEWORD(1,1), &wsa_data);
    #endif

    if (argc / 2 >= 1)
    {
        for (int i = 0; i < argc; i++)
        {
            if (strcmp(argv[i], "--ip") == 0)
            {
                struct addrinfo hints, *res;

                memset (&hints, 0, sizeof (hints));
                hints.ai_family = AF_UNSPEC;
                hints.ai_socktype = SOCK_STREAM;
                hints.ai_protocol = IPPROTO_TCP;

                if (getaddrinfo(argv[i + 1], NULL, &hints, &res) == 0)
                {
                    g.server_addr.sin_addr = ((struct sockaddr_in *) res->ai_addr)->sin_addr;
                    g.online = 1;
                }
                else
                {
                    printf("Couldn't resolve the server hostname.\n");
                }
            }
            else if (strcmp(argv[i], "--port") == 0)
            {
                g.server_addr.sin_port = htons((unsigned short) atoi(argv[i + 1]));
            }
            else if (strcmp(argv[i], "--nick") == 0)
            {
                strcpy(g.player_nickname, argv[i + 1]);
            }
        }
    }

    game_init(&g, window);
    input_init(&i, window);

    const double tick_interval = 1.0 / 20.0;
    const double frame_interval = 1.0 / FPS;
    double last_time = 0.0;
    double current_time = 0.0;
    double delta_time = 0.0;
    double tick_accumulator = tick_interval;

    int first_frame = 1;

    while (!glfwWindowShouldClose(window))
    {
        current_time = glfwGetTime();
        if (first_frame)
        {
            last_time = current_time - frame_interval;
            first_frame = 0;
        }
        delta_time = current_time - last_time;

        if (delta_time < frame_interval && !first_frame)
        {
            usleep(frame_interval * 1000000 - delta_time * 1000000);
            current_time = glfwGetTime();
            delta_time = current_time - last_time;
        }

        tick_accumulator += delta_time;

        while (tick_accumulator >= tick_interval)
        {
            game_tick(&g);
            tick_accumulator -= tick_interval;
        }

        input_poll_events(&i);
        game_handle_input(&g, &i);

        game_draw(&g, delta_time, tick_accumulator);

        glfwSwapBuffers(window);
        last_time = current_time;
    }

    game_destroy(&g);

    glfwDestroyWindow(window);
    glfwTerminate();

    #ifdef _WIN32
        WSACleanup();
    #endif

    return 0;
}
