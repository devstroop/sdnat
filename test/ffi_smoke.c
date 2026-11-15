#include "sdnat.h"

#include <stdint.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int main(void) {
    if (sdnat_version() == NULL || sdnat_version()[0] == '\0') return 10;
    if (sdnat_create(NULL) != NULL) return 11;
    if (sdnat_create("not json") != NULL) return 12;

    const char *config =
        "{\"version\":1,\"instance\":\"ffi-smoke\",\"log\":{\"level\":\"err\"},"
        "\"ports\":{\"vpn\":{\"kind\":\"l3\",\"device\":\"vpn0\"}}}";
    sdnat_t handle = sdnat_create(config);
    if (handle == NULL) return 13;

    sdnat_status_t status;
    memset(&status, 0, sizeof(status));
    if (sdnat_get_status(handle, &status) != SDNAT_OK) return 14;
    if (status.struct_size != sizeof(status)) return 15;
    if (status.abi_version != SDNAT_ABI_VERSION) return 16;
    if (status.running != 0 || status.port_count != 0) return 17;

    int fds[2];
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, fds) != 0) return 18;
    if (sdnat_set_port_fd(handle, "vpn", fds[0]) != SDNAT_OK) return 19;
    if (sdnat_set_event_cb(handle, NULL, NULL) != SDNAT_OK) return 20;
    if (sdnat_start(handle) != SDNAT_OK) return 21;
    if (close(fds[0]) != 0) return 22;

    if (sdnat_get_status(handle, &status) != SDNAT_OK) return 23;
    if (status.running != 1 || status.port_count != 1) return 24;

    if (sdnat_stop(handle) != SDNAT_OK) return 25;
    if (sdnat_get_status(handle, &status) != SDNAT_OK) return 26;
    if (status.running != 0) return 27;

    sdnat_destroy(handle);
    if (close(fds[1]) != 0) return 28;
    return 0;
}
