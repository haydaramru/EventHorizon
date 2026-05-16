#define _POSIX_C_SOURCE 199309L

#include "structured_log.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define DEFAULT_LOG_PATH "/var/log/eventhorizon/eventhorizon.jsonl"

static FILE *log_file = NULL;
static int log_disabled = 0;

static const char *value_or_none(const char *value) {
    return value ? value : "none";
}

static void write_json_string(FILE *out, const char *value) {
    const unsigned char *cursor = (const unsigned char *)value_or_none(value);

    fputc('"', out);
    while (*cursor) {
        switch (*cursor) {
            case '"':
                fputs("\\\"", out);
                break;
            case '\\':
                fputs("\\\\", out);
                break;
            case '\n':
                fputs("\\n", out);
                break;
            case '\r':
                fputs("\\r", out);
                break;
            case '\t':
                fputs("\\t", out);
                break;
            default:
                fputc(*cursor, out);
                break;
        }
        cursor++;
    }
    fputc('"', out);
}

static void write_json_field(FILE *out, const char *name, const char *value) {
    fputc(',', out);
    write_json_string(out, name);
    fputc(':', out);
    write_json_string(out, value);
}

static void utc_timestamp(char *buffer, size_t buffer_len) {
    struct timespec ts;
    struct tm tm_utc;

    if (clock_gettime(CLOCK_REALTIME, &ts) != 0) {
        snprintf(buffer, buffer_len, "1970-01-01T00:00:00.000Z");
        return;
    }

    if (!gmtime_r(&ts.tv_sec, &tm_utc)) {
        snprintf(buffer, buffer_len, "1970-01-01T00:00:00.000Z");
        return;
    }

    snprintf(
        buffer,
        buffer_len,
        "%04d-%02d-%02dT%02d:%02d:%02d.%03ldZ",
        tm_utc.tm_year + 1900,
        tm_utc.tm_mon + 1,
        tm_utc.tm_mday,
        tm_utc.tm_hour,
        tm_utc.tm_min,
        tm_utc.tm_sec,
        ts.tv_nsec / 1000000L
    );
}

void structured_log_init(const char *path) {
    const char *env_path = getenv("EVENTHORIZON_LOG_PATH");
    const char *log_path = path;

    if (env_path && env_path[0] != '\0') {
        log_path = env_path;
    } else if (!log_path) {
        log_path = DEFAULT_LOG_PATH;
    }

    log_file = fopen(log_path, "a");
    if (!log_file) {
        fprintf(stderr, "structured_log: failed to open %s: %s\n", log_path, strerror(errno));
        log_disabled = 1;
        return;
    }

    log_disabled = 0;
}

void structured_log_event(
    const char *event_type,
    const char *protocol,
    const char *session_id,
    const char *source_ip,
    const char *mode,
    const char *persona,
    const char *strategy,
    long long duration_ms,
    const char *disconnect_reason,
    int delay_ms
) {
    char ts[32];

    if (log_disabled || !log_file) {
        return;
    }

    utc_timestamp(ts, sizeof(ts));

    fputc('{', log_file);
    write_json_string(log_file, "ts");
    fputc(':', log_file);
    write_json_string(log_file, ts);
    write_json_field(log_file, "event_type", event_type);
    write_json_field(log_file, "protocol", protocol);
    write_json_field(log_file, "session_id", session_id);
    write_json_field(log_file, "source_ip", source_ip);
    write_json_field(log_file, "mode", mode);
    write_json_field(log_file, "persona", persona);
    write_json_field(log_file, "strategy", strategy);

    if (duration_ms != -1) {
        fprintf(log_file, ",\"duration_ms\":%lld", duration_ms);
    }
    if (disconnect_reason) {
        write_json_field(log_file, "disconnect_reason", disconnect_reason);
    }
    if (delay_ms != -1) {
        fprintf(log_file, ",\"delay_ms\":%d", delay_ms);
    }

    fputs("}\n", log_file);
    fflush(log_file);
}
