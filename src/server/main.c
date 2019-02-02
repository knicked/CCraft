#include "server.h"

#include "GLFW/glfw3.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    printf("[CCraft 0.0.0 Server]\n");

    server s;
    unsigned short port = 25565;
    if (argc / 2 >= 1)
    {
        for (int i = 0; i < argc; i++)
        {
            if (strcmp(argv[i], "--port") == 0)
            {
                port = (unsigned short) atoi(argv[i + 1]);
            }
        }
    }
    server_init(&s, port);

    double last_time = 0.0;
    double current_time = 0.0;
    double delta_time = 0.0;
    double tick_accumulator = 0.0;
    const double tick_interval = 1.0 / 20.0;

    int first_tick = 1;

    struct timeval tv;
    fd_set read_fds;

    tv.tv_sec = 0;
    tv.tv_usec = 0;

    while (1)
    {
        current_time = glfwGetTime();
        if (first_tick)
        {
            last_time = current_time - tick_interval;
            first_tick = 0;
        }
        delta_time = current_time - last_time;

        if (delta_time < tick_interval && !first_tick)
        {
            usleep(tick_interval * 1000000 - delta_time * 1000000);
            current_time = glfwGetTime();
            delta_time = current_time - last_time;
        }

        last_time = current_time;

        tick_accumulator += delta_time;

        server_tick(&s);

        FD_ZERO(&read_fds);
        FD_SET(0, &read_fds);

        select(1, &read_fds, NULL, NULL, &tv);

        if (FD_ISSET(0, &read_fds))
            break;
    }

    printf("Closing the server...\n");
    server_destroy(&s);

    return 0;
}
