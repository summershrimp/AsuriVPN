//
// Created by ubuntu on 12/29/16.
//
#include <sys/resource.h>
#include <errno.h>

#include "event.h"
#include "utils.h"
#include "common.h"

int epoll_fd = -1;
int event_loop_running = 0;

int event_init_loop(int max_fd) {
    struct rlimit rt;
    rt.rlim_max = rt.rlim_cur = (rlim_t)max_fd + 1;
    if(setrlimit(RLIMIT_NOFILE, &rt) == -1) {
        perror("setrlimit() - RLIMIT_NOFILE");
        exit(errno);
    }
    epoll_fd = epoll_create(max_fd);
    if(epoll_fd <= 0) {
        perror("epoll_create() - max_fd");
        exit(errno);
    }
    return 0;
}

int event_start_loop() {
    event_loop_running = 1;
    struct epoll_event *events;
    events = (struct epoll_event *)malloc(sizeof(struct epoll_event) * EVENTS_ONCE);
    int event_cnt, i;
    struct event *e;
    while(event_loop_running != 0) {
        event_cnt = epoll_wait(epoll_fd, events, EVENTS_ONCE, -1);
        if(event_cnt == -1) {
            perror("epoll_wait()");
            exit(errno);
        }
        for(i=0; i < event_cnt; ++i) {
            e = (struct event *)events[i].data.ptr;
            e->handler(*e);
        }
    }
    free(events);
    return 0;
}

int event_stop_loop() {
    event_loop_running = 0;
    return event_loop_running;
}

int event_add(struct event *e, uint32_t events) {
    struct epoll_event ev;
    ev.events = events;
    ev.data.ptr = (void *)e;
    //make_nonblock(e->fd);
    if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, e->fd, &ev) == -1) {
        perror("epoll_ctl() - EPOLL_CTL_ADD");
        exit(errno);
    }
    return 0;
}

int event_delete(struct event *e, uint32_t events) {
    struct epoll_event ev;
    ev.events = events;
    ev.data.ptr = (void *)e;
    if(epoll_ctl(epoll_fd, EPOLL_CTL_DEL, e->fd, &ev) == -1) {
        perror("epoll_ctl() - EPOLL_CTL_DEL");
        return 1;
    }
    return 0;
}

