# Release

SDNAT uses semantic versioning. The Linux preview target is **0.2.0**; Android
is a separate milestone.

## Gate before tagging

Do not push a `v*` tag until all of these are true:

- `zig build test`, `zig build web-test` and the netns scripts pass.
- 72 h fuzzing is clean (`zig build fuzz --fuzz=<iterations>` or a dedicated
  campaign).
- `scripts/soak.sh --duration 24h` passes with no traffic loss.
- `scripts/perf.sh --scenario all` meets the CORE-SCOPE §12 budgets on a named
  reference host. The 64 B packet-rate budget is currently not met; see
  [PERFORMANCE.md](PERFORMANCE.md).
- A fresh package install passes [QUICKSTART.md](QUICKSTART.md) and
  `sdnat down`/doctor restores the host.

The 0.2.0 version in `build.zig.zon` is a development version. It is not a
claim that the gates above are green.

## Build packages

```sh
ZIG=/path/to/zig bash scripts/package.sh --rpm
```

Outputs in `dist/`:

- `sdnat-<version>-linux-<arch>.tar.gz`
- `sdnat_<version>_<arch>.deb`
- `sdnat-<version>-1.<rpmarch>.rpm` when `--rpm` is passed
- `SHA256SUMS-<arch>`

Cross-build the ARM64 tarball and Debian package:

```sh
ZIG=/path/to/zig bash scripts/package.sh --target aarch64-linux-gnu
```

Cross-building an RPM for a foreign architecture is not supported by
`rpmbuild` on a foreign host; build the ARM64 RPM on an ARM64 runner.

`scripts/package.sh` builds a Debian package with `ar` and `tar`; it does not
need `dpkg-deb`. Pass `--rpm` to also build an RPM when `rpmbuild` is
installed. Install the Debian package with `sudo apt install ./dist/sdnat_*.deb`,
or the RPM with `sudo rpm -i ./dist/sdnat-*.rpm`. The tarball contains the same
rootfs layout under `sdnat-<version>/` and can be unpacked over `/` after
reviewing the file list.

The package installs:

- `/usr/bin/sdnat`, `/usr/bin/sdnatd`
- `/usr/lib/libsdnat.so*`, `/usr/lib/libsdnat.a`, `/usr/include/sdnat.h`
- `/lib/systemd/system/sdnatd.service`
- `/etc/sdnat/sdnat.json.example`
- `/usr/share/doc/sdnat/` (this guide, quickstart, troubleshooting,
  performance report, changelog, license and examples)

It does not enable or start `sdnatd`; create `/etc/sdnat/sdnat.json` first.

## Release workflow

`.github/workflows/release.yml` runs on a `v*` tag. It checks that the tag
matches `build.zig.zon`, builds `amd64` and `arm64` packages on native runners,
and attaches the tarballs, Debian packages, RPMs and a combined `SHA256SUMS` to
a GitHub release.

To prepare a release:

```sh
# bump build.zig.zon and CHANGELOG.md first
git tag -a v0.2.0 -m "SDNAT 0.2.0 Linux preview"
git push origin v0.2.0
```

The workflow does not create a release for a branch or pull request.

## Install and upgrade

```sh
sudo apt install ./sdnat_0.2.0_amd64.deb
sudo cp /etc/sdnat/sdnat.json.example /etc/sdnat/sdnat.json
sudoedit /etc/sdnat/sdnat.json
sudo sdnat validate /etc/sdnat/sdnat.json
sudo systemctl enable --now sdnatd
sudo journalctl -u sdnatd -b
```

On upgrade, stop the daemon first if the old package changed ownership of
interfaces or addresses. The journal is tied to the instance name and run
directory; do not rename either during an upgrade without a migration plan.

## Roll back

```sh
sudo systemctl stop sdnatd
sudo apt install ./sdnat_<previous>_<arch>.deb
sudo sdnat doctor --repair -c /etc/sdnat/sdnat.json
```

If a crash or failed upgrade left host state behind, inspect `sdnat journal`
and run `sdnat doctor --repair` before restarting the new version.

## Known release blockers

- 64 B forwarding does not meet 1 Mpps; the datapath is single-packet and
  single-queue.
- 24 h soak and 72 h fuzz evidence must be captured for the exact tag.
- GSO frames from local veth peers are not reassembled; the bridge does not
  repair checksum-partial frames. See
  [TROUBLESHOOTING.md](TROUBLESHOOTING.md).
- AppImage is not produced by the current script; the tarball, `.deb` and
  `.rpm` are validated. AppImage remains an R9a packaging gap, and a daemon
  unit is a better fit for system integration than an AppImage.
