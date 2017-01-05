//
// Created by ubuntu on 12/29/16.
//

#include "server.h"
#include "client.h"
#include "event.h"
#include "config.h"
#include "common.h"

int main(int argc, char **argv) {
    //make_daemon();
    config_init();
    event_init_loop(fd_max);
    if(!(server ^ client)) {
        fputs("Cannot run in both server mode and client mode", stderr);
        exit(-1);
    }
    if(client == 1) {
        fputs("Client mode\n", stderr);
        client_init();
    } else if( server == 1) {
        fputs("Server mode\n", stderr);
        server_init();
    }

    event_start_loop();
    return 0;
}

