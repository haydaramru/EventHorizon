#ifndef STRUCTURED_LOG_H
#define STRUCTURED_LOG_H

void structured_log_init(const char *path);

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
);

#endif
