//
// Created by ubuntu on 1/5/17.
//

#ifndef ASURIVPN_LOGGER_H
#define ASURIVPN_LOGGER_H
enum _log_level {LOG_DEBUG = 1, LOG_INFO, LOG_WARN, LOG_ERROR };

void log_set_level(enum _log_level level);
void log_error(const char* message, ...);
void log_warn(const char* message, ...);
void log_debug(const char* message, ...);
void log_info(const char* message, ...);
#endif //ASURIVPN_LOGGER_H
