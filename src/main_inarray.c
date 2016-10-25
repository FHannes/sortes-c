/**
 * This implementation of the SORTES C language fluency assignment implements malloc() and free(),
 * using no memory space beyond that provided by the workarea array.
 */

#include <stdio.h>

#ifndef size_t
    #define size_t unsigned int
#endif
#ifndef null
    #define null (void*) 0
#endif

#define WORK_AREA_SIZE 1048576

static char workarea[WORK_AREA_SIZE];
static void* workarea_start = workarea;
static void* workarea_end = workarea + WORK_AREA_SIZE - 1;

typedef struct memory_chunks {
    struct memory_chunks* previous;
    struct memory_chunks* next;
    size_t size;
} memory_chunk;
memory_chunk* first_chunk;
size_t memory_chunk_size = sizeof(memory_chunk);

/**
 * Prints a linked list of memory chunks.
 *
 * @param first The first chunk in the list
 */
void dump_chunks(memory_chunk* first) {
    printf("===================== Chunk dump =====================\n");
    printf("         CHUNK        PREVIOUS           NEXT     SIZE\n");
    for (memory_chunk* c = first; (void*) c < (void*) workarea_end; c = c->next) {
        printf("%14p: %14p %14p %8u\n", c, c->previous, c->next, c->size);
        size_t gap = (size_t) ((void*) c->next - (void*) c - memory_chunk_size - c->size);
        if (gap != 0) {
            printf("Gap: %8u bytes\n", gap);
        }
    }
}

/**
 * Initializes the memory space used by malloc2().
 */
void init() {
    first_chunk = workarea_start;
    first_chunk->previous = null;
    first_chunk->next = workarea_start + WORK_AREA_SIZE;
    first_chunk->size = 0;
}

/**
 * Allocates memory in the workarea array.
 *
 * @param size The amount of bytes which should be allocated
 * @return A pointer to the allocated memory or a nullpointer in case the memory could not be allocated
 */
void* malloc2(size_t size) {
    if (size == 0) {
        return null;
    }

    void* chunk_pointer = workarea_start;
    while (chunk_pointer <= workarea_end - memory_chunk_size) {
        memory_chunk* chunk = chunk_pointer;
        int first_empty = chunk == first_chunk && chunk->size == 0;
        if (((void*)chunk->next) - chunk_pointer - (first_empty ? 1 : 2) * memory_chunk_size - chunk->size >= size) {
            void* new_offset = first_empty ? first_chunk : chunk_pointer + memory_chunk_size + chunk->size;

            memory_chunk* new_pointer = new_offset;
            new_pointer->next = chunk->next;
            new_pointer->size = size;

            if ((void*) chunk->next < workarea_end) {
                chunk->next->previous = new_pointer;
            }

            if (new_pointer != first_chunk) {
                new_pointer->previous = chunk_pointer;
                chunk->next = new_offset;
            }

            return new_offset + memory_chunk_size;
        }
        chunk_pointer = chunk->next;
    }
    return null;
}

/**
 * Frees a chunk of memory allocated by the malloc2() function.
 *
 * @param mem A pointer to the memory area which should be freed
 */
void free2(void* mem) {
    if (mem != null) {
        memory_chunk* chunk = mem - memory_chunk_size;
        chunk->size = 0;
        if (chunk->previous != null) {
            chunk->previous->next = chunk->next;
        }
        if ((void*) chunk->next < workarea_end) {
            chunk->next->previous = chunk->previous;
        }
    }
}

/**
 * The entry-point for the application.
 */
void main() {
    init();

    // Test allocation of first memory chunk
    malloc2(1);
    dump_chunks(first_chunk);

    // Test allocation of additional chunk
    malloc2(100);
    dump_chunks(first_chunk);

    // Test freeing memory
    void* mem = malloc2(10);
    dump_chunks(first_chunk);
    free2(mem);
    dump_chunks(first_chunk);

    // Test allocating more memory than available
    malloc2(WORK_AREA_SIZE);
    dump_chunks(first_chunk);

    // Test freeing null
    free2(null);

    // Test maximum allocation
    init();
    malloc2(WORK_AREA_SIZE - memory_chunk_size);
    dump_chunks(first_chunk);

    // Test more complex series of operations
    init();
    void* mem1 = malloc2(100);
    void* mem2 = malloc2(100);
    void* mem3 = malloc2(100);
    malloc2(100);
    free2(mem2);
    free2(mem3);
    malloc2(200);
    free2(mem1);
    malloc2(50);
    malloc2(50);
    malloc2(50 - memory_chunk_size);
    dump_chunks(first_chunk);
}
