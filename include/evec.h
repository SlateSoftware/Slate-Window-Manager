/* easyvec - a simple vector implementation for C */
/* made by <franzageek> on the go */

#ifndef __EVEC_H__
#define __EVEC_H__ 1

#include <intdef.h>
#include <stdlib.h>
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
    free(evec);
}

evec_t evec__new(u16 size_bytes)
{
    evec_t evec = {0};
    u8* data = calloc( 8, size_bytes);
    if (!data)
        return evec;

    evec.data = data;
    evec.capacity = 8;
    evec.size = 0;
    evec.item_size = size_bytes;
    evec.valid = true;
    data = NULL;
    return evec;
}

u8 evec__push(evec_t* evec, void* data)
{
    if (evec->size >= evec->capacity)
    {
        u8* new_data = (u8*)calloc(evec->capacity + (evec->capacity / 2), evec->item_size);
        if (!new_data)
            return 1;
        
        memmove(new_data, evec->data, evec->capacity * evec->item_size);
        if (memcmp(new_data, evec->data, evec->capacity * evec->item_size) != 0)
            return 1;

        free(evec->data);
        evec->data = new_data;
        new_data = NULL;
        evec->capacity += evec->capacity / 2;
    }
    u8* dest = &evec->data[evec->size * evec->item_size];
    memmove(dest, data, evec->item_size);
    if (memcmp(data, dest, evec->item_size) != 0)
        return 1;
    
    evec->size += 1;
    return 0;
}

void* evec__at(evec_t* evec, u16 index)
{
    if (index < evec->item_size)
        return (void*)&evec->data[index * evec->item_size];
    
    else
        return NULL;
}

#endif