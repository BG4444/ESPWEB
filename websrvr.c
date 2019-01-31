#include "websrvr.h"

#include "ets_sys.h"
#include "osapi.h"
#include "os_type.h"
#include "user_interface.h"
#include "espconn.h"
#include "mem.h"
#include "gpio.h"
#include "tcp_streamer.h"
#include "tar.h"
#include <stdlib.h>
#include "logger.h"

tcp_streamer* streamsOut = 0;
tcp_streamer* streamsInp = 0;

void answer500(struct espconn *conn,const size_t code)
{
    strBuf bufcode;

    intToBuf(500+code,&bufcode);
    strBuf buf;

    strBuf statusBuf;

    append(2, &statusBuf, &bufcode, &status500);

    makeHTTPReply(&statusBuf,0,0,&buf);

    log_free(statusBuf.begin);
    log_free(bufcode.begin);

    sendStringCreateStreamerNoCopy(&streamsOut,conn,&buf);
}

bool unpackParams(const strBuf *params, Param *ret, const unsigned short count)
{
    unsigned short int i=0;
    for(;i < count; ++i)
    {
        strBuf nameValue[2];
        if(!split(&params[i], nameValue, 2, '='))
        {
            return false;
        }

        ret[i].name=nameValue[0];

        if(nameValue[1].len!=1)
        {
            return false;
        }

        const char tValue=nameValue[1].begin[0];

        if(tValue <'0' || tValue > '1')
        {
            return false;
        }
        ret[i].value=tValue-'0';
    }
    return true;
}

unsigned short findParam(const strBuf *name, const Param *params, const unsigned short count)
{
    unsigned short int i=0;
    for(; i< count; ++i)
    {
        if(compare(name,&params[i].name))
        {
            return i;
        }
    }
    return m1;
}


void makeHTTPReply(const strBuf *statusString, const size_t contentSize, const strBuf* etag, strBuf* out)
{
    MAKE_STR_BUF(HTTPReplyHead,"HTTP/1.1 ");
    MAKE_STR_BUF(xtimeStrBegin,"X-Time: \"");
    MAKE_STR_BUF(etagStrBegin,"ETag: \"");
    MAKE_STR_BUF(etagStrEnd,"\"");
    MAKE_STR_BUF(HTTPStrEnd,"\r\n");
    MAKE_STR_BUF(replyBegin,"Server: ESPWEB\r\nConnection: Keep-Alive\r\nContent-Length: ");

    strBuf contentLength;

    intToBuf(contentSize,&contentLength);

    strBuf xtime;

    intToBuf(nTicks,&xtime);

    if(etag)
    {
        append(15, out, &HTTPReplyHead,
                        statusString,
                        &HTTPStrEnd, &replyBegin, &contentLength,
                        &HTTPStrEnd, &etagStrBegin,  etag, &etagStrEnd,
                        &HTTPStrEnd, &xtimeStrBegin,&xtime,&etagStrEnd,
                        &HTTPStrEnd,
                        &HTTPStrEnd

               );
    }
    else
    {
        append(11, out, &HTTPReplyHead, statusString, &HTTPStrEnd, &replyBegin, &contentLength,&HTTPStrEnd,&xtimeStrBegin,&xtime,&etagStrEnd, &HTTPStrEnd, &HTTPStrEnd);
    }

    log_free(contentLength.begin);
    log_free(xtime.begin);
}
