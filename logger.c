#include "logger.h"
#include "mem.h"
#include "osapi.h"

log_entry* log_entries=0;

size_t mem_usage=0;
size_t nTicks=0;

void add_message_copy(const strBuf *mess, const size_t time)
{
    strBuf tmess;
    copy(mess,&tmess);
    add_message(&tmess,time);
}

void add_message(const strBuf *mess, const size_t time)
{
    log_entry* entry = add_log_entry_item(&log_entries);

    entry ->message=*mess;
    entry ->timestamp=time;

    char buf[512];
    os_sprintf(buf,"Memory free is %u\n",system_get_free_heap_size());


    strBuf newClient={buf,strlen(buf)};

    entry = add_log_entry_item(&log_entries);

    entry ->timestamp=time;

    copy(&newClient,&entry->message);

    const size_t mlen=(mess->len > 511) ? 511 : mess->len;

    memcpy(buf, mess->begin, mlen);

    buf[mlen]=0;

    os_printf(buf);
}



size_t getCurrentLength()
{
    size_t ret=0;
    for(log_entry* pos=log_entries; pos; pos=pos->next)
    {
        ret+=pos->message.len;
    }
    return ret;
}

size_t getCurrentDepth()
{
    size_t ret=0;
    for(log_entry* pos=log_entries; pos; pos=pos->next)
    {
        ++ret;
    }
    return ret;
}


void registerMemoryGet(size_t size)
{
//    mem_usage+=size;
//    char buf[512];
//    os_sprintf(buf,"Memory free is %u",system_get_free_heap_size());
//    add_log_buffer(buf);
}



void add_log_buffer(const char *buf)
{
    strBuf newClient={buf,strlen(buf)};
    add_message_copy(&newClient,nTicks);
}



void *log_malloc(const size_t size)
{
    registerMemoryGet(size);
    return os_malloc(size);
}



void log_free(void *entry)
{
    registerMemoryGet(0);
    os_free(entry);
}

ADD_ITEM(log_entry)
DELETE_ITEM(log_entry)
