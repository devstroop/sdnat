# Quickstart

This guide runs SDNAT as a Linux router with two packet ports, a LAN DHCP
server and source NAT on the uplink. Replace `eth0`/`eth1` with your devices.

## Requirements

- Linux with `AF_PACKET` support
- root or `CAP_NET_ADMIN`, `CAP_NET_RAW` and (for DHCP/DNS) `CAP_NET_BIND_SERVICE`
- Zig 0.16.x to build from source
- `iproute2` for inspection

## Build and test

```sh
zig build --release=fast
zig build test
```

The binaries are `zig-out/bin/sdnat` and `zig-out/bin/sdnatd`. The release
package installs the same files under `/usr/bin`.

## Install a package

```sh
sudo apt install ./sdnat_0.2.0_amd64.deb
# or: sudo rpm -i sdnat-0.2.0-1.x86_64.rpm
# or unpack dist/sdnat-0.2.0-linux-amd64.tar.gz over /
```

The package installs the systemd unit but does not start the daemon until
`/etc/sdnat/sdnat.json` exists:

```sh
sudo cp /etc/sdnat/sdnat.json.example /etc/sdnat/sdnat.json
sudoedit /etc/sdnat/sdnat.json
sudo sdnat validate /etc/sdnat/sdnat.json
sudo systemctl enable --now sdnatd
```

Check it:

```sh
sudo sdnat status -c /etc/sdnat/sdnat.json
sudo journalctl -u sdnatd -f
```

## Example configuration

```json
{
  "version": 1,
  "instance": "router",
  "ports": {
    "wan": { "kind": "l2", "device": "eth0" },
    "lan": { "kind": "l2", "device": "eth1" }
  },
  "interfaces": {
    "wan0": { "port": "wan", "address": "dhcp" },
    "lan0": { "port": "lan", "address": "10.0.0.1/24" }
  },
  "dhcp": [
    { "iface": "lan0", "pool": "10.0.0.100-10.0.0.200" }
  ],
  "nat": {
    "masquerade": [ { "out": "wan0" } ]
  },
  "firewall": {
    "default_in": "accept",
    "default_forward": "accept",
    "default_out": "accept"
  }
}
```

For a static uplink, replace `"address": "dhcp"` with the upstream address and
add a default route in the configuration.

## Run from the build tree

```sh
./zig-out/bin/sdnat validate examples/minimal.json
sudo ./zig-out/bin/sdnatd -c examples/minimal.json
```

Use a second terminal:

```sh
sudo ./zig-out/bin/sdnat status -c examples/minimal.json
```

## Move a port to a bridge

A port belongs to at most one bridge or router zone. An SVI is a separate
logical interface attached to the bridge:

```json
{
  "ports": {
    "lan1": { "kind": "l2", "device": "eth1" },
    "lan2": { "kind": "l2", "device": "eth2" }
  },
  "bridges": {
    "br0": { "ports": ["lan1", "lan2"] }
  },
  "interfaces": {
    "br0svi": { "bridge": "br0", "address": "10.0.0.1/24" }
  }
}
```

## Stop and clean up

Normal stop:

```sh
sudo systemctl stop sdnatd
```

If the daemon crashed or was killed, replay the journal:

```sh
sudo sdnat doctor -c /etc/sdnat/sdnat.json
sudo sdnat doctor --repair -c /etc/sdnat/sdnat.json
```

`sdnat down` is the CLI teardown path when no daemon is running. Always inspect
`sdnat journal` before repairing an unfamiliar system.

## Next steps

- [Configuration and scope](CORE-SCOPE.md)
- [Troubleshooting](TROUBLESHOOTING.md)
- [Performance](PERFORMANCE.md)
- [Release process](RELEASE.md)
