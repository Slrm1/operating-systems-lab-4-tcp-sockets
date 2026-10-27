#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 9001
#define MAX_COMMAND_LINE_LEN 1024

static char *getCommandLine(char *command_line) {
    do {
        if (fgets(command_line, MAX_COMMAND_LINE_LEN, stdin) == NULL) {
            if (ferror(stdin)) perror("fgets");
            return NULL;
        }
    } while (command_line[0] == '\n');
    command_line[strcspn(command_line, "\n")] = 0;
    return command_line;
}

static void print_menu(void) {
    puts("COMMANDS:\n---------\n1. print\n2. get_length\n3. add_back <value>\n4. add_front <value>\n5. add_position <index> <value>\n6. remove_back\n7. remove_front\n8. remove_position <index>\n9. get <index>\n10. exit");
}

int main(void) {
    int socket_fd;
    struct sockaddr_in server_address;
    char command[MAX_COMMAND_LINE_LEN];
    char responseData[MAX_COMMAND_LINE_LEN];

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) { perror("socket"); return EXIT_FAILURE; }
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    if (connect(socket_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("connect"); close(socket_fd); return EXIT_FAILURE;
    }

    while (1) {
        char command_copy[MAX_COMMAND_LINE_LEN];
        char *name;
        ssize_t received;
        int exiting;

        printf("Enter Command (or menu): ");
        fflush(stdout);
        if (getCommandLine(command) == NULL) break;
        if (strcmp(command, "menu") == 0) { print_menu(); continue; }
        if (command[0] == 0) continue;

        snprintf(command_copy, sizeof(command_copy), "%s", command);
        name = strtok(command_copy, " ");
        exiting = name != NULL && strcmp(name, "exit") == 0;

        if (send(socket_fd, command, strlen(command), 0) < 0) {
            perror("send"); break;
        }
        memset(responseData, 0, sizeof(responseData));
        received = recv(socket_fd, responseData, sizeof(responseData) - 1, 0);
        if (received < 0) { perror("recv"); break; }
        if (received == 0) { fprintf(stderr, "Server disconnected.\n"); break; }
        responseData[received] = 0;
        printf("\nSERVER RESPONSE: %s\n", responseData);
        if (exiting) break;
    }

    if (close(socket_fd) < 0 && errno != EINTR) perror("close");
    return EXIT_SUCCESS;
}
