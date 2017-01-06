//
// Created by ubuntu on 12/29/16.
//

#ifndef ASURIVPN_EVENT_H
#define ASURIVPN_EVENT_H

#include <sys/epoll.h>
#define EVENTS_ONCE 100

enum event_type {EVENT_TUN = 1, EVENT_SOCKET};
struct event;
typedef int (*event_handler) (struct event *e);
struct event {
    enum event_type type;
    int fd;
    event_handler handler;
    void *ptr;
};

extern int epoll_fd;
extern int event_loop_running;

int event_init_loop(int max_fd);
int event_start_loop();
int event_stop_loop();
int event_add(struct event *e, uint32_t events);
int event_delete(struct event *e, uint32_t events);

#endif //ASURIVPN_EVENT_H
