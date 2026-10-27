#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "list.h"

#define PORT 9001
#define ACK "ACK "
#define BUFFER_SIZE 1024

static volatile sig_atomic_t stop_requested = 0;
static int server_fd = -1;
static int client_fd = -1;
static list_t *active_list = NULL;

static void cleanup(void) {
    if (client_fd >= 0) { close(client_fd); client_fd = -1; }
    if (server_fd >= 0) { close(server_fd); server_fd = -1; }
    if (active_list != NULL) { list_free(active_list); active_list = NULL; }
}

static void handle_signal(int signal_number) {
    (void)signal_number;
    stop_requested = 1;
}

static int parse_int(char *token, int *value) {
    char *end;
    long parsed;
    if (token == NULL) return 0;
    errno = 0;
    parsed = strtol(token, &end, 10);
    if (errno != 0 || *end != 0) return 0;
    *value = (int)parsed;
    return 1;
}

static int process_command(char *buffer, char *response, size_t capacity) {
    char *command = strtok(buffer, " ");
    char *first;
    char *second;
    int value;
    int index;

    if (command == NULL) {
        snprintf(response, capacity, "ERROR: invalid command or argument");
    } else if (strcmp(command, "exit") == 0) {
        snprintf(response, capacity, "BYE");
        return 0;
    } else if (strcmp(command, "print") == 0) {
        snprintf(response, capacity, "%s", listToString(active_list));
    } else if (strcmp(command, "get_length") == 0) {
        snprintf(response, capacity, "Length = %d", list_length(active_list));
    } else if (strcmp(command, "add_back") == 0) {
        first = strtok(NULL, " ");
        if (!parse_int(first, &value)) snprintf(response, capacity, "ERROR: invalid command or argument");
        else { list_add_to_back(active_list, value); snprintf(response, capacity, "%s%d", ACK, value); }
    } else if (strcmp(command, "add_front") == 0) {
        first = strtok(NULL, " ");
        if (!parse_int(first, &value)) snprintf(response, capacity, "ERROR: invalid command or argument");
        else { list_add_to_front(active_list, value); snprintf(response, capacity, "%s%d", ACK, value); }
    } else if (strcmp(command, "add_position") == 0) {
        first = strtok(NULL, " ");
        second = strtok(NULL, " ");
        if (!parse_int(first, &index) || !parse_int(second, &value)) snprintf(response, capacity, "ERROR: invalid command or argument");
        else { list_add_at_index(active_list, value, index); snprintf(response, capacity, "%s%d", ACK, value); }
    } else if (strcmp(command, "remove_back") == 0) {
        value = list_remove_from_back(active_list);
        snprintf(response, capacity, "%s%d", ACK, value);
    } else if (strcmp(command, "remove_front") == 0) {
        value = list_remove_from_front(active_list);
        snprintf(response, capacity, "%s%d", ACK, value);
    } else if (strcmp(command, "remove_position") == 0) {
        first = strtok(NULL, " ");
        if (!parse_int(first, &index)) snprintf(response, capacity, "ERROR: invalid command or argument");
        else { value = list_remove_at_index(active_list, index); snprintf(response, capacity, "%s%d", ACK, value); }
    } else if (strcmp(command, "get") == 0) {
        first = strtok(NULL, " ");
        if (!parse_int(first, &index)) snprintf(response, capacity, "ERROR: invalid command or argument");
        else { value = list_get_elem_at(active_list, index); snprintf(response, capacity, "%d", value); }
    } else {
        snprintf(response, capacity, "ERROR: invalid command or argument");
    }
    return 1;
}

int main(void) {
    struct sockaddr_in address;
    int reuse = 1;
    char buffer[BUFFER_SIZE];
    char response[BUFFER_SIZE];

    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);
    active_list = list_alloc();
    if (active_list == NULL) { perror("list_alloc"); return EXIT_FAILURE; }

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) { perror("socket"); cleanup(); return EXIT_FAILURE; }
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(PORT);
    address.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0 ||
        listen(server_fd, 1) < 0) {
        perror("server setup"); cleanup(); return EXIT_FAILURE;
    }

    client_fd = accept(server_fd, NULL, NULL);
    if (client_fd < 0) {
        if (!stop_requested) perror("accept");
        cleanup(); return stop_requested ? EXIT_SUCCESS : EXIT_FAILURE;
    }

    while (!stop_requested) {
        ssize_t received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        if (received <= 0) break;
        buffer[received] = 0;
        memset(response, 0, sizeof(response));
        int keep_running = process_command(buffer, response, sizeof(response));
        if (send(client_fd, response, strlen(response) + 1, 0) < 0) break;
        if (!keep_running) break;
    }

    cleanup();
    return EXIT_SUCCESS;
}
