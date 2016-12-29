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
    if(argc > 1 && argv[1][0] == 'c') {
        fputs("Client mode\n", stderr);
        config_client();
        client_init();
    } else {
        fputs("Server mode\n", stderr);
        server_init();
    }

    event_start_loop();
    return 0;
}

