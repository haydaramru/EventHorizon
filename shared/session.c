#include "session.h"

#include <stdio.h>

void make_session_id(
    char *buffer,
    int buffer_len,
    const char *protocol,
    const char *source_ip,
    long long started_at_ms
) {
    if (!buffer || buffer_len <= 0) {
        return;
    }

    snprintf(
        buffer,
        (size_t)buffer_len,
        "%s-%s-%lld",
        protocol ? protocol : "unknown",
        source_ip ? source_ip : "unknown",
        started_at_ms
    );
}
