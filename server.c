#include <stdio.h>          // printf(), perror(), fwrite()
#include <stdlib.h>         // General utilities
#include <unistd.h>         // close(), read()
#include <sys/socket.h>     // Socket functions
#include <netinet/in.h>     // IPv4 structures/functions
#include <string.h>         // strlen(), strchr()
#include <fcntl.h>          // open()

int main(void)
{
    int server_fd;                  // Listening socket
    int client_fd;                  // Connected client socket

    char buffer[1024];              // Temporary buffer for HTTP request

    struct sockaddr_in address;     // IPv4 address and port


    // Create a TCP/IPv4 socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd == -1) {
        perror("socket");
        return 1;
    }


    // Configure the server address
    address.sin_family = AF_INET;          // IPv4
    address.sin_addr.s_addr = INADDR_ANY;  // Listen on all interfaces
    address.sin_port = htons(8080);        // Port 8080


    // Attach socket to address and port
    if (bind(server_fd,
             (struct sockaddr *)&address,
             sizeof(address)) == -1) {

        perror("bind");
        close(server_fd);
        return 1;
    }


    // Start listening for connections
    if (listen(server_fd, 10) == -1) {

        perror("listen");
        close(server_fd);
        return 1;
    }


    // Keep accepting clients
    while (1)
    {
        // Wait for a client
        client_fd = accept(server_fd, NULL, NULL);

        if (client_fd == -1) {
            perror("accept");
            continue;
        }

        printf("Client connected: %d\n", client_fd);


        // Receive the HTTP request
        ssize_t bytes_received;

        bytes_received = recv(
            client_fd,
            buffer,
            sizeof(buffer),
            0
        );

        if (bytes_received == -1) {
            perror("recv");
            close(client_fd);
            continue;
        }

        printf("Received %zd bytes\n", bytes_received);


        // Print the received request
        fwrite(buffer, 1, bytes_received, stdout);


        // Find the first space after the HTTP method
        char *method_end = strchr(buffer, ' ');

        if (method_end != NULL) {

            // Move to the requested path
            char *path = method_end + 1;

            // Find the end of the path
            char *path_end = strchr(path, ' ');

            if (path_end != NULL) {

                // End the path string
                *path_end = '\0';

                printf("Path: %s\n", path);


                // Open the requested file
                int file_fd = open(path + 1, O_RDONLY);

                if (file_fd == -1) {
                    perror("open");
                } else {

                    printf("File opened: %d\n", file_fd);


                    // Buffer for file contents
                    char file_buffer[4096];


                    // Read file into memory
                    ssize_t bytes_read = read(
                        file_fd,
                        file_buffer,
                        sizeof(file_buffer)
                    );


                    if (bytes_read == -1) {
                        perror("read");
                    } else {

                        printf("Read %zd bytes\n", bytes_read);

                        // Print file contents
                        fwrite(
                            file_buffer,
                            1,
                            bytes_read,
                            stdout
                        );
                    }


                    // Close the file
                    close(file_fd);
                }
            }
        }


        // Create a simple HTTP response
        const char *response =
            "HTTP/1.1 200 OK\r\n"
            "Content-Length: 12\r\n"
            "Content-Type: text/plain\r\n"
            "\r\n"
            "Hello World\n";


        // Send the response
        send(
            client_fd,
            response,
            strlen(response),
            0
        );


        // Close the client connection
        close(client_fd);
    }

    return 0;
}
