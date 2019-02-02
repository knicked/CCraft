#include "server.h"

#include <stdio.h>

void server_init(server *s, unsigned short port)
{
    #ifdef _WIN32
        WSADATA wsa_data;
        WSAStartup(MAKEWORD(1,1), &wsa_data);
    #endif

    FD_ZERO(&s->sockets);

    struct sockaddr_in addrport;
    s->listener = socket(PF_INET, SOCK_STREAM, 0);
    addrport.sin_family = AF_INET;
    addrport.sin_port = htons(port);
    addrport.sin_addr.s_addr = htonl(INADDR_ANY);

    if (SOCKET_VALID(bind(s->listener, (struct sockaddr *) &addrport, sizeof(addrport))))
    {
        if (SOCKET_VALID(listen(s->listener, 10)))
        {
            printf("Server listening for connections on port %d\n", port);
            FD_SET(s->listener, &s->sockets);
            s->max_fd = s->listener;
        }
        else
            printf("Server socket can't listen for connections.\n");
    }
}

void server_tick(server *s)
{
    fd_set read_fds = s->sockets;
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    if (SOCKET_VALID(select(s->max_fd + 1, &read_fds, NULL, NULL, &tv)))
    {
        for (int i = 0; i <= s->max_fd; i++)
        {
            if (FD_ISSET(i, &read_fds))
            {
                if (i == s->listener)
                {
                    struct sockaddr_storage client_addr;
                    socklen_t addr_len = sizeof(client_addr);
                    SOCKET new_fd = accept(s->listener, (struct sockaddr *) &client_addr, &addr_len);
                    FD_SET(new_fd, &s->sockets);
                    if (new_fd > s->max_fd) s->max_fd = new_fd;
                    printf("New connection!\n");
                }
                else
                {
                    int data_size;
                    if (SOCKET_VALID(data_size = recv(i, s->buffer, sizeof(s->buffer), 0)))
                    {
                        if (data_size == 0)
                        {
                            printf("Client disconnected.\n");
                            close(i);
                            FD_CLR(i, &s->sockets);
                        }
                        else
                        {
                            if (data_size == sizeof(set_block_packet))
                            {
                                for (int j = 0; j <= s->max_fd; j++)
                                {
                                    if (j != s->listener && j != i)
                                    {
                                        send(j, s->buffer, sizeof(set_block_packet), 0);
                                    }
                                }
                                set_block_packet *packet = (set_block_packet *) s->buffer;
                                printf("The block at %d/%d/%d has been set to %d\n", (short) ntohs(packet->x), (short) ntohs(packet->y), (short) ntohs(packet->z), packet->block);
                            }
                        }
                    }
                }
            }
        }
    }
}

void server_destroy(server *s)
{
    close(s->listener);
}
