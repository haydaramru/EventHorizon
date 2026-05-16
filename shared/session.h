#ifndef SESSION_H
#define SESSION_H

void make_session_id(
    char *buffer,
    int buffer_len,
    const char *protocol,
    const char *source_ip,
    long long started_at_ms
);

#endif
