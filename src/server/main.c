#include "server.h"

#include <sys/time.h>
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

    struct timeval current_tv;
    gettimeofday(&current_tv, NULL);

    unsigned int last_time = 0;
    unsigned int current_time = 0;
    unsigned int delta_time = 0;
    unsigned int tick_accumulator = 0;
    const unsigned int tick_interval = 1000000 / 20;

    int first_tick = 1;

    struct timeval tv;
    fd_set read_fds;

    tv.tv_sec = 0;
    tv.tv_usec = 0;

    while (1)
    {
        gettimeofday(&current_tv, NULL);
        current_time = 1000000 * current_tv.tv_sec + current_tv.tv_usec;
        if (first_tick)
        {
            last_time = current_time - tick_interval;
            first_tick = 0;
        }
        delta_time = current_time - last_time;

        if (delta_time < tick_interval && !first_tick)
        {
            usleep(tick_interval - delta_time);
            gettimeofday(&current_tv, NULL);
            current_time = 1000000 * current_tv.tv_sec + current_tv.tv_usec;
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
