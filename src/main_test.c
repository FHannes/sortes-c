/**
 * This implementation of the SORTES C language test.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum { false = 0, true } bool;

typedef struct bucket_type {
    char data[64];
    char length;
    struct bucket_type* next;
} bucket;

bool clear_bucket(bucket* ptr) {
    if (ptr == NULL) {
        return false;
    }

    ptr->length = 0;
    ptr->next = NULL;

    return true;
}

bool alloc_bucket(bucket** ptr) {
    if (ptr == NULL) {
        return false;
    }

    *ptr = malloc(sizeof(bucket));
    if (*ptr == NULL) {
        return false;
    }

    return clear_bucket(*ptr);
}

int savetext(char* text, bucket** anchor, bucket** freelist) {
    if (anchor == NULL) {
        // No list provided to store the text
        return -1;
    }
    
    int total = 0;
    if (*anchor != NULL) {
        while ((*anchor)->length == 64 && (*anchor)->next != NULL) {
            (*anchor) = (*anchor)->next;
            total += 64;
        }
        total += (*anchor)->length;
    }

    int length = strlen(text);
    if (length == 0) {
        // Text is empty, do nothing
        return total;
    }

    bucket* list = *anchor;
    while (length > 0) {
        if (list == NULL || list->length == 64) {
            bucket* new;
            if (freelist != NULL && *freelist != NULL) {
                new = *freelist;
                *freelist = new->next;
                
                if (!clear_bucket(new)) {
                    return -1;
                }
            } else if (!alloc_bucket(&new)) {
                return -1;
            }
            if (list != NULL) {
                list->next = new;
            } else {
                *anchor = new;
            }
            list = new;
        }

        int remaining = 64 - list->length;
        int amount = (length > remaining ? remaining : length);
        strncpy(&list->data[list->length], text, amount);
        list->length += amount;
        text += amount;
        length -= amount;

        total += amount;
    }
    
    return total;
}

char* readdoc(bucket* doc) {
    if (doc == NULL) {
        // No document provided
        return (char*) -1;
    }

    // Get document length
    int length = 0;
    bucket* tmp = doc;
    while (tmp != NULL) {
        length += tmp->length;
        tmp = tmp->next;
    }

    // Allocate string
    char* doc_string = malloc(length + 1);
    if (doc_string == NULL) {
        return (char*) -1;
    }

    // Build document string
    tmp = doc;
    int current = 0;
    while (tmp != NULL) {
        strncpy(&doc_string[current], tmp->data, tmp->length);
        current += tmp->length;
        tmp = tmp->next;
    }
    doc_string[length] = 0;

    return doc_string;
}

char* getdoc(bucket** doc, bucket** freelist) {
    if (doc == NULL) {
        // No document provided
        return (char*) -1;
    }

    // Get document length
    int length = 0;
    bucket* tmp = *doc;
    while (tmp != NULL) {
        length += tmp->length;
        tmp = tmp->next;
    }

    // Allocate string
    char* doc_string = malloc(length + 1);
    if (doc_string == NULL) {
        return (char*) -1;
    }

    // Get freelist length
    bucket* tmp_freelist = NULL;
    if (freelist != NULL) {
        tmp_freelist = *freelist;
        while (tmp_freelist != NULL) {
            tmp_freelist = tmp_freelist->next;
        }
    }

    // Build document string
    tmp = *doc;
    int current = 0;
    while (tmp != NULL) {
        strncpy(&doc_string[current], tmp->data, tmp->length);
        current += tmp->length;

        bucket* old = tmp;
        tmp = tmp->next;

        clear_bucket(old);
        if (tmp_freelist == NULL) {
            tmp_freelist = old;
            *freelist = old;
        } else {
            tmp_freelist->next = old;
            tmp_freelist = old;
        }
    }
    doc_string[length] = 0;

    // The freelist should not have more than 10 buckets after running getdoc
    tmp_freelist = NULL;
    if (freelist != NULL) {
        tmp_freelist = *freelist;
        int free_count = 1;
        while (tmp_freelist != NULL) {
            bucket* old = tmp_freelist;
            tmp_freelist = tmp_freelist->next;
            if (free_count == 10) {
                free(old);
            } else {
                free_count++;
                if (free_count == 9) {
                    old->next = NULL;
                }
            }
        }
    }

    *doc = NULL;

    return doc_string;
}

int main() {
    bucket* doclist = NULL;
    bucket* freelist = NULL;

    printf("========= RUN 1 =========\n");

    if (savetext("Hello ", &doclist, &freelist) == -1) {
        printf("Error in savetext\n");
        return 0;
    }
    if (savetext("world!", &doclist, &freelist) == -1) {
        printf("Error in savetext\n");
        return 0;
    }
    if (savetext("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaab", &doclist, &freelist) == -1) {
        // b is the 65th character
        printf("Error in savetext\n");
        return 0;
    }

    char* doc = readdoc(doclist);
    if (doc == (char*) -1) {
        printf("Error in readdoc\n");
        return 0;
    }
    printf("%s\n", doc);
    printf("doc %p | free %p\n", doclist, freelist);
    free(doc);

    doc = getdoc(&doclist, &freelist);
    if (doc == (char*) -1) {
        printf("Error in getdoc\n");
        return 0;
    }
    printf("%s\n", doc);
    printf("doc %p | free %p\n", doclist, freelist);
    free(doc);

    printf("\n========= RUN 2 =========\n");

    if (savetext("Hello ", &doclist, &freelist) == -1) {
        printf("Error in savetext\n");
        return 0;
    }
    if (savetext("world!", &doclist, &freelist) == -1) {
        printf("Error in savetext\n");
        return 0;
    }
    if (savetext("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaab", &doclist, &freelist) == -1) {
        // b is the 65th character
        printf("Error in savetext\n");
        return 0;
    }

    doc = readdoc(doclist);
    if (doc == (char*) -1) {
        printf("Error in readdoc\n");
        return 0;
    }
    printf("%s\n", doc);
    printf("doc %p | free %p\n", doclist, freelist);
    free(doc);

    doc = getdoc(&doclist, &freelist);
    if (doc == (char*) -1) {
        printf("Error in getdoc\n");
        return 0;
    }
    printf("%s\n", doc);
    printf("doc %p | free %p\n", doclist, freelist);
    free(doc);

    // Clean up lists
    while (doclist != NULL) {
        bucket* old = doclist;
        doclist = doclist->next;
        free(old);
    }
    while (freelist != NULL) {
        bucket* old = freelist;
        freelist = freelist->next;
        free(old);
    }
}
