

#include <stdio.h>
#ifndef size_t
    #define size_t unsigned int
#endif
#ifndef null
    #define null (void*) 0
#endif

#define MAX_ALLOC 100
#define WORK_AREA_SIZE 1048576

#define CHUNK_META_SIZE MAX_ALLOC * 2 + 1

static char workarea[WORK_AREA_SIZE];
static void* workarea_start = workarea;
static void* workarea_end = workarea + WORK_AREA_SIZE - 1;

typedef struct memory_chunks {
    struct memory_chunks* next;
    void* location;
    size_t size;
    unsigned char allocated;
    unsigned char in_use;
} memory_chunk;
static memory_chunk chunks[CHUNK_META_SIZE];

/**
 * Prints a linked list of memory chunks.
 *
 * @param first The first chunk in the list
 */
void dump_chunks(memory_chunk* first) {
    printf("======================= Chunk dump =======================\n");
    printf("         CHUNK        LOCATION           NEXT    SIZE FREE\n");
    for (memory_chunk* c = chunks; c != null; c = c->next) {
        printf("%14p: %14p %14p %7u %4u\n", c, c->location, c->next, c->size, !c->allocated);
    }
}

void init() {
    memory_chunk* chunk = chunks;
    for (int chunk_idx = 0; chunk_idx < CHUNK_META_SIZE; chunk_idx++) {
        chunk->next = null;
        chunk->in_use = 0;
        chunk++;
    }

    chunks->next = null;
    chunks->location = workarea;
    chunks->size = WORK_AREA_SIZE;
    chunks->allocated = 0;
    chunks->in_use = 1;
}

void* malloc2(size_t size) {
    for (memory_chunk* chunk = chunks; chunk != null; chunk = chunk->next) {
        // Find the first unallocated chunk with sufficient memory space
        if (!chunk->allocated && chunk->size >= size) {
            if (chunk->size == size) {
                // The free chunk is a perfect match!
                chunk->allocated = 1;
                return chunk->location;
            } else {
                // Find unused chunk data memory
                memory_chunk* new_chunk = chunks;
                int chunk_idx = 0;
                for (; chunk_idx < CHUNK_META_SIZE; chunk_idx++) {
                    if (new_chunk->in_use == 0) {
                        break;
                    }
                    new_chunk++;
                }

                // No free chunk data memory, can't allocate
                if (chunk_idx == CHUNK_META_SIZE) {
                    return null;
                }

                // Create a new chunk to handle the remaining memory of the old chunk which was not allocated
                new_chunk->next = chunk->next;
                new_chunk->location = chunk->location + size;
                new_chunk->size = chunk->size - size;
                new_chunk->allocated = 0;
                new_chunk->in_use = 1;

                // Resize the old chunk and allocate it
                chunk->next = new_chunk;
                chunk->size = size;
                chunk->allocated = 1;

                return chunk->location;
            }
        }
    }
    return null;
}

void free2(void* mem) {
    memory_chunk* previous = null;
    for (memory_chunk* chunk = chunks; chunk != null; chunk = chunk->next) {
        // Find the chunk which starts at the given memory pointer
        if (chunk->location == mem) {
            // Release memory allocation
            chunk->allocated = 0;

            // Merge with next chunk if it also free memory
            memory_chunk* next = chunk->next;
            if (next != null && next->allocated == 0) {
                chunk->next = next->next;
                chunk->size += next->size;
                next->next = null;
                next->in_use = 0;
            }

            // Merge with previous chunk if it also free memory
            if (previous != null && previous->allocated == 0) {
                previous->next = chunk->next;
                previous->size += chunk->size;
                chunk->next = null;
                chunk->in_use = 0;
            }
            
            return;
        }
        previous = chunk;
    }
}

void main() {
    init();

    // Test allocation of first memory chunk
    malloc2(1);
    dump_chunks(chunks);

    // Test allocation of additional chunk
    malloc2(100);
    dump_chunks(chunks);

    // Test freeing memory
    void* mem = malloc2(10);
    dump_chunks(chunks);
    free2(mem);
    dump_chunks(chunks);

    // Test allocating more memory than available
    malloc2(WORK_AREA_SIZE);
    dump_chunks(chunks);

    // Test freeing null
    free2(null);

    // Test maximum allocation
    init();
    malloc2(WORK_AREA_SIZE);
    dump_chunks(chunks);

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
    dump_chunks(chunks);
}