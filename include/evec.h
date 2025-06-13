/* easyvec - a simple vector implementation for C */
/* made by <franzageek> on the go */

#ifndef __EVEC_H__
#define __EVEC_H__ 1

#include <intdef.h>
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

typedef struct _evec
{
    u8* data;
    u8 capacity;
    u8 size;
    u8 item_size;
    bool valid;
} evec_t;

void evec__free(evec_t* evec)
{
    free(evec->data);
    evec->data = 0; 
    evec->capacity = 0;
    evec->size = 0;
    evec->item_size = 0;
    evec->valid = 0;
}

evec_t evec__new(u16 size_bytes)
{
    evec_t evec = {0};
    u8* data = calloc( 8, size_bytes);
    if (!data)
    {
        perror("evec: calloc:");
        exit(EXIT_FAILURE);
    }

    evec.data = data;
    evec.capacity = 8;
    evec.size = 0;
    evec.item_size = size_bytes;
    evec.valid = true;
    data = NULL;
    return evec;
}

void evec__push(evec_t* evec, void* data)
{
    if (evec->valid)
    {
        if (evec->size >= evec->capacity)
        {
            u8* new_data = (u8*)calloc(evec->capacity + (evec->capacity / 2), evec->item_size);
            if (!new_data)
            {
                perror("evec: calloc:");
                evec__free(evec);
                exit(EXIT_FAILURE);
            }
            
            memmove(new_data, evec->data, evec->capacity * evec->item_size);
            if (memcmp(new_data, evec->data, evec->capacity * evec->item_size) != 0)
            {
                fprintf(stderr, "evec: an error occurred after memmove() call: source and destination do not match\n");
                evec__free(evec);
                exit(EXIT_FAILURE);
            }
    
            free(evec->data);
            evec->data = new_data;
            new_data = NULL;
            evec->capacity += evec->capacity / 2;
        }
        u8* dest = &evec->data[evec->size * evec->item_size];
        memmove(dest, data, evec->item_size);
        if (memcmp(data, dest, evec->item_size) != 0)
        {
            fprintf(stderr, "evec: an error occurred after memmove() call: source and destination do not match\n");
            evec__free(evec);
            exit(EXIT_FAILURE);
        }
        
        evec->size += 1;
        return;
    }
    else
    {
        fprintf(stderr, "evec: an error occurred after evec__push(): called on an invalid evec\n");
        exit(EXIT_FAILURE);
    }    
}

void* evec__at(evec_t* evec, u16 index)
{
    if (evec->valid)
    {
        if (index < evec->item_size)
            return (void*)&evec->data[index * evec->item_size];
        
        else
        {
            fprintf(stderr, "evec: attempt to read out of range\n");
            evec__free(evec);
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        fprintf(stderr, "evec: an error occurred after evec__at(): called on an invalid evec\n");
        exit(EXIT_FAILURE);
    }
}

#endif