#include "list.h"
#include <stdio.h>
#include <stdlib.h>

list_t *list_alloc(void) {
    list_t *list = malloc(sizeof(*list));
    if (list != NULL) list->head = NULL;
    return list;
}

void list_free(list_t *list) {
    if (list == NULL) return;
    node_t *node = list->head;
    while (node != NULL) {
        node_t *next = node->next;
        free(node);
        node = next;
    }
    free(list);
}

static node_t *new_node(int value) {
    node_t *node = malloc(sizeof(*node));
    if (node == NULL) return NULL;
    node->value = value;
    node->next = NULL;
    return node;
}

void list_add_to_front(list_t *list, int value) {
    if (list == NULL) return;
    node_t *node = new_node(value);
    if (node == NULL) return;
    node->next = list->head;
    list->head = node;
}

void list_add_to_back(list_t *list, int value) {
    if (list == NULL) return;
    node_t *node = new_node(value);
    if (node == NULL) return;
    if (list->head == NULL) { list->head = node; return; }
    node_t *cur = list->head;
    while (cur->next != NULL) cur = cur->next;
    cur->next = node;
}

int list_length(list_t *list) {
    int length = 0;
    if (list == NULL) return 0;
    for (node_t *cur = list->head; cur != NULL; cur = cur->next) ++length;
    return length;
}

void list_add_at_index(list_t *list, int value, int index) {
    if (list == NULL || index < 0 || index > list_length(list)) return;
    if (index == 0) { list_add_to_front(list, value); return; }
    node_t *node = new_node(value);
    if (node == NULL) return;
    node_t *cur = list->head;
    for (int i = 1; i < index; ++i) cur = cur->next;
    node->next = cur->next;
    cur->next = node;
}

int list_remove_from_front(list_t *list) {
    if (list == NULL || list->head == NULL) return -1;
    node_t *old = list->head;
    int value = old->value;
    list->head = old->next;
    free(old);
    return value;
}

int list_remove_from_back(list_t *list) {
    if (list == NULL || list->head == NULL) return -1;
    if (list->head->next == NULL) return list_remove_from_front(list);
    node_t *cur = list->head;
    while (cur->next->next != NULL) cur = cur->next;
    int value = cur->next->value;
    free(cur->next);
    cur->next = NULL;
    return value;
}

int list_remove_at_index(list_t *list, int index) {
    if (list == NULL || index < 0 || index >= list_length(list)) return -1;
    if (index == 0) return list_remove_from_front(list);
    node_t *cur = list->head;
    for (int i = 1; i < index; ++i) cur = cur->next;
    node_t *old = cur->next;
    int value = old->value;
    cur->next = old->next;
    free(old);
    return value;
}

int list_get_elem_at(list_t *list, int index) {
    if (list == NULL || index < 0) return -1;
    node_t *cur = list->head;
    for (int i = 0; cur != NULL && i < index; ++i) cur = cur->next;
    return cur == NULL ? -1 : cur->value;
}

char *listToString(list_t *list) {
    static char output[4096];
    size_t used = 0;
    output[0] = 0;
    if (list == NULL) return output;
    for (node_t *cur = list->head; cur != NULL && used < sizeof(output); cur = cur->next) {
        int written = snprintf(output + used, sizeof(output) - used,
                               cur->next == NULL ? "%d" : "%d -> ", cur->value);
        if (written < 0 || (size_t)written >= sizeof(output) - used) break;
        used += (size_t)written;
    }
    return output;
}
