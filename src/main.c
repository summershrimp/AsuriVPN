//
// Created by ubuntu on 12/29/16.
//

#include "server.h"
#include "client.h"
#include "event.h"
#include "config.h"
#include "common.h"
#include "analyze.h"

int main(int argc, char **argv) {
    //make_daemon();
    logfile = stderr;
    config_init();

    SSL_library_init();
    OpenSSL_add_ssl_algorithms();
    SSL_load_error_strings();

    event_init_loop(fd_max);
    if(!(server ^ client)) {
        log_error("Cannot run in both server mode and client mode");
        exit(-1);
    }
    if(client == 1) {
        log_info("Client mode");
        client_init();
    } else if( server == 1) {
        log_info("Server mode");
        server_init();
    }

    event_start_loop();
    return 0;
}

