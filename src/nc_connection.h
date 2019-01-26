/*
 * twemproxy - A fast and lightweight proxy for memcached protocol.
 * Copyright (C) 2011 Twitter, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _NC_CONNECTION_H_
#define _NC_CONNECTION_H_

#include <nc_core.h>

typedef rstatus_t (*conn_recv_t)(struct context *, struct conn*);
typedef struct msg* (*conn_recv_next_t)(struct context *, struct conn *, bool);
typedef void (*conn_recv_done_t)(struct context *, struct conn *, struct msg *, struct msg *);

typedef rstatus_t (*conn_send_t)(struct context *, struct conn*);
typedef struct msg* (*conn_send_next_t)(struct context *, struct conn *);
typedef void (*conn_send_done_t)(struct context *, struct conn *, struct msg *);

typedef void (*conn_close_t)(struct context *, struct conn *);
typedef bool (*conn_active_t)(struct conn *);

typedef void (*conn_ref_t)(struct conn *, void *);
typedef void (*conn_unref_t)(struct conn *);

typedef void (*conn_msgq_t)(struct context *, struct conn *, struct msg *);
typedef void (*conn_post_connect_t)(struct context *ctx, struct conn *, struct server *server);
typedef void (*conn_swallow_msg_t)(struct conn *, struct msg *, struct msg *);

struct conn {
    TAILQ_ENTRY(conn)   conn_tqe;        /* link in server_pool / server / free q */
    void                *owner;          /* connection owner - server_pool / server */

    int                 sd;              /* socket descriptor */
    int                 family;          /* socket address family */
    socklen_t           addrlen;         /* socket length */
    struct sockaddr     *addr;           /* socket address (ref in server or server_pool) */

    //conn 对应的incoming request queue, 会从conn中读取数据, 组织成msg写入到该queue中
    struct msg_tqh      imsg_q;          /* incoming request Q */

    //conn 对应的output queue, 需要向该conn写的msg会push到该queue中
    struct msg_tqh      omsg_q;          /* outstanding request Q */
    //每次重新接受一个完整的请求时, rmsg为空, 如果某次读取只读取了请求的一部分,则rmsg
    // 不为空,下次读取的数据继续追加到当前rmsg中
    struct msg          *rmsg;           /* current message being rcvd */
    struct msg          *smsg;           /* current message being sent */

    conn_recv_t         recv;            /* recv (read) handler */ //读事件触发时回调
    conn_recv_next_t    recv_next;       /* recv next message handler */ //实际读数据之前，调这个函数来得到当前正在使用的msg
    conn_recv_done_t    recv_done;       /* read done handler */ //每次接受一个完整的消息后,回调
    conn_send_t         send;            /* send (write) handler */ //写时间触发时的回调
    conn_send_next_t    send_next;       /* write next message handler */ //实际写数据之前, 定位当前要写的msg
    conn_send_done_t    send_done;       /* write done handler */ //发送完一个msg, 回调一次
    conn_close_t        close;           /* close handler */
    conn_active_t       active;          /* active? handler */
    conn_post_connect_t post_connect;    /* post connect handler */
    conn_swallow_msg_t  swallow_msg;     /* react on messages to be swallowed */

    conn_ref_t          ref;             /* connection reference handler */ //得到一个连接后,将连接加入到相应的队列中
    conn_unref_t        unref;           /* connection unreference handler */

    //这四个队列用来存放msg指针, 和zero copy的实现密切相关
    conn_msgq_t         enqueue_inq;     /* connection inq msg enqueue handler */
    conn_msgq_t         dequeue_inq;     /* connection inq msg dequeue handler */
    conn_msgq_t         enqueue_outq;    /* connection outq msg enqueue handler */
    conn_msgq_t         dequeue_outq;    /* connection outq msg dequeue handler */

    size_t              recv_bytes;      /* received (read) bytes */
    size_t              send_bytes;      /* sent (written) bytes */

    uint32_t            events;          /* connection io events */
    err_t               err;             /* connection errno */
    unsigned            recv_active:1;   /* recv active? */ //标志位,标识当前conn已经加入到epoll的监测中
    unsigned            recv_ready:1;    /* recv ready? */
    unsigned            send_active:1;   /* send active? */
    unsigned            send_ready:1;    /* send ready? */

    unsigned            client:1;        /* client? or server? */ //conn的类型, client和proxy之间:1, proxy和server之间:0
    unsigned            proxy:1;         /* proxy? */
    unsigned            connecting:1;    /* connecting? */
    unsigned            connected:1;     /* connected? */
    unsigned            eof:1;           /* eof? aka passive close? */
    unsigned            done:1;          /* done? aka close? */
    unsigned            redis:1;         /* redis? */ //后端server是redis还是memcached
    unsigned            authenticated:1; /* authenticated? */
};

TAILQ_HEAD(conn_tqh, conn);

struct context *conn_to_ctx(struct conn *conn);
struct conn *conn_get(void *owner, bool client, bool redis);
struct conn *conn_get_proxy(void *owner);
void conn_put(struct conn *conn);
ssize_t conn_recv(struct conn *conn, void *buf, size_t size);
ssize_t conn_sendv(struct conn *conn, struct array *sendv, size_t nsend);
void conn_init(void);
void conn_deinit(void);
uint32_t conn_ncurr_conn(void);
uint64_t conn_ntotal_conn(void);
uint32_t conn_ncurr_cconn(void);
bool conn_authenticated(struct conn *conn);

#endif
