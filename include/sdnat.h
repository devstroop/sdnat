#ifndef SDNAT_H
#define SDNAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SDNAT_ABI_VERSION 4u

/*
 * The native library uses the host's POSIX threads. Android bionic provides
 * them; Linux consumers of an older libc should link with -pthread.
 */

typedef void *sdnat_t;

enum {
    SDNAT_OK = 0,
    SDNAT_ERR_INVALID_ARGUMENT = -1,
    SDNAT_ERR_INVALID_CONFIG = -2,
    SDNAT_ERR_OUT_OF_MEMORY = -3,
    SDNAT_ERR_BUSY = -4,
    SDNAT_ERR_UNSUPPORTED = -5,
    SDNAT_ERR_PERMISSION = -6,
    SDNAT_ERR_IO = -7,
    SDNAT_ERR_NOT_FOUND = -8,
    SDNAT_ERR_INTERNAL = -9
};

enum {
    SDNAT_EVENT_STARTED = 1,
    SDNAT_EVENT_STOPPED = 2,
    SDNAT_EVENT_ERROR = 3
};

typedef struct sdnat_status {
    uint32_t struct_size;
    uint32_t abi_version;
    uint32_t running;
    uint32_t port_count;
    uint64_t uptime_ms;
} sdnat_status_t;

typedef void (*sdnat_event_cb)(void *user, int event, const char *message);

/*
 * Create an instance from a complete JSON config. Returns NULL when the JSON
 * is malformed or fails validation. The input buffer is not retained.
 *
 * Hosts may configure an existing L3 port (kind "l3" without "create") and
 * inject its descriptor with sdnat_set_port_fd before sdnat_start.
 */
sdnat_t sdnat_create(const char *config_json);

/* Starts the data plane. Blocks until startup succeeds or fails. */
int sdnat_start(sdnat_t handle);

/* Stops the data plane and joins the data-plane thread. Idempotent. */
int sdnat_stop(sdnat_t handle);

/* Stops if needed and releases all native resources. */
void sdnat_destroy(sdnat_t handle);

/*
 * Bind a host-owned descriptor to a configured L3 port.
 *
 * The descriptor is borrowed: SDNAT duplicates it during sdnat_start, so
 * the caller keeps ownership and may close it after sdnat_start returns.
 * It must remain valid for every sdnat_start call. A stopped handle reuses
 * the same bindings when restarted. Call before sdnat_start.
 *
 * SDNAT sets O_NONBLOCK on the duplicate. Since dup() shares the open file
 * description, the original descriptor also becomes non-blocking.
 */
int sdnat_set_port_fd(sdnat_t handle, const char *name, int fd);

/*
 * Install a lifecycle callback. Events run on the data-plane thread. STARTED
 * is delivered before sdnat_start returns. Do not call sdnat_* functions
 * other than sdnat_get_status from inside the callback; sdnat_stop and
 * sdnat_destroy would deadlock. Keep callbacks short. `message` may be NULL
 * and is valid only for the duration of the callback.
 */
int sdnat_set_event_cb(sdnat_t handle, sdnat_event_cb callback, void *user);

/* Fill a caller-owned status snapshot. */
int sdnat_get_status(sdnat_t handle, sdnat_status_t *status);

/* Returns a static NUL-terminated version string. */
const char *sdnat_version(void);

#ifdef __cplusplus
}
#endif

#endif
