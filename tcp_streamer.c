#include "mem.h"
#include "tcp_streamer.h"

ADD_ITEM(tcp_streamer);
DELETE_ITEM(tcp_streamer);

bool is_sending = false;
struct espconn* current_sending;

tcp_streamer *find_item(tcp_streamer *current,struct  espconn *pCon)
{
    for(;current && current->pEspCon!=pCon; current=current->next);
    return current;
}

bool compare_socket(tcp_streamer* a, struct  espconn *b)
{
    os_printf("compare ip's\n");
    for(size_t i = 0; i<4;++i)
    {
        os_printf("%d %d\n",a->remote_ip[i],b->proto.tcp->remote_ip[i]);
        if(a->remote_ip[i] != b->proto.tcp->remote_ip[i])
        {
            os_printf("MISSED IP!\n");
            return false;
        }
    }
    os_printf("compare ports\n");
    if(
            a->remote_port == b->proto.tcp->remote_port
        &&
            a->local_port  == b->proto.tcp->local_port)
    {        
        return true;
    }
    return false;
}

tcp_streamer *find_socket(tcp_streamer *current,struct  espconn *pCon)
{
    for(;current ; current=current->next)
    {
        os_printf("checking node %d %d",current,pCon);
        if(compare_socket(current,pCon))
        {
            break;
        }
    }
    return current;
}

void setCon(tcp_streamer *s,struct espconn * conn)
{
    s->pEspCon=conn;
    s->remote_port=conn->proto.tcp->remote_port;
    s->local_port=conn->proto.tcp->local_port;

    for(size_t i=0;i<4;++i)
    {
     s->remote_ip[i]=conn->proto.tcp->remote_ip[i];
    }
}

void sendStringCreateStreamer(tcp_streamer **current, struct espconn* conn, const strBuf *buffer)
{
    tcp_streamer* s = add_tcp_streamer_item(current);

    setCon(s,conn);

    sendString(s,buffer);
}

void sendStringCreateStreamerNoCopy(tcp_streamer **current, struct espconn* conn, const strBuf *buffer)
{
    tcp_streamer* s = add_tcp_streamer_item(current);

    setCon(s,conn);

    sendStringNoCopy(s,buffer);
}


void sendString(tcp_streamer* s,const strBuf *buffer)
{
    if(is_sending)
    {
        s->mode=SendString;
        copy(buffer,&s->string);
    }
    else
    {
        s->mode=KillMeNoDisconnect;
        is_sending=true;
        os_printf("send tcpstreamer item");
        current_sending=s->pEspCon;
        espconn_sent(s->pEspCon,buffer->begin,buffer->len);
        log_free(buffer->begin);
    }
}


void sendStringNoCopy(tcp_streamer *s, const strBuf *buffer)
{
    if(is_sending)
    {
        s->mode=SendString;
        s->string=*buffer;
    }
    else
    {
        s->mode=KillMeNoDisconnect;
        is_sending=true;
        os_printf("send tcp streamer item 2");
        current_sending=s->pEspCon;
        espconn_sent(s->pEspCon,buffer->begin,buffer->len);
        log_free(buffer->begin);
    }
}

void sendFileNoCopy(tcp_streamer *s, strBuf *buffer, uint32_t pos, uint32_t tail)
{
    if(is_sending)
    {
        s->mode=SendFile;

        s->string=*buffer;

        s->pos=pos;
        s->tail=tail;
    }
    else
    {
        s->mode=File;

        s->pos=pos;
        s->tail=tail;

        is_sending=true;
        os_printf("send tcp streamer item 3");
        current_sending=s->pEspCon;
        espconn_sent(s->pEspCon,buffer->begin,buffer->len);

        log_free(buffer->begin);
    }
}

void print(tcp_streamer *in)
{
    os_printf("+++\n",in);
    for(;in;in=in->next)
    {
        os_printf("%d\n",in);
    }
    os_printf("---\n",in);
}
