# ADR-019 — Rename vNET to SDNAT

- **Status:** Accepted
- **Date:** 2026-10-27
- **Follows:** [ADR-017](017-project-rename-vnet.md)

## Context

ADR-017 renamed vRouter to vNET when the product outgrew the router label.
The name has since been reconsidered: the project's differentiator and the
use case that drives most of the configuration, testing and documentation is
software-defined NAT for Linux and Android hosts (tethering, VPN/proxy modes,
port forwarding, hairpin). Leading with "virtual network" undersells that
identity, and `vNet` collides visually with common virtual-network
abbreviations and Azure's spelling.

The project is pre-1.0 with no released tags, installed C consumers or frozen
ABI. The cost of renaming now is a mechanical source change; the cost after the
ABI and packaging stabilize would be permanent aliases or a breaking release.

## Decision

Use **SDNAT** in prose and `sdnat` in commands, identifiers, paths and package
names. The casing is deliberate: uppercase `SDNAT` reads as an acronym and is
distinctive; lowercase `sdnat` is used wherever a machine reads the name.

1. **Product and documentation.** The product is SDNAT; use lowercase `sdnat`
   anywhere a machine reads the name.
2. **Executables and artifacts.** `vnet`, `vnetd` and `vnetweb` become
   `sdnat`, `sdnatd` and `sdnatweb`. Build module and package names use
   `sdnat`; libraries are `libsdnat` and `libsdnat.a`.
3. **C ABI.** `include/vnet.h` becomes `include/sdnat.h`; exported functions,
   types, enum values and macros become `sdnat_*` / `SDNAT_*`. The status struct
   layout and lifecycle semantics are unchanged, but the symbol namespace
   change is a breaking ABI event, so `SDNAT_ABI_VERSION` becomes 4.
4. **Paths and environment.** Defaults become `/run/sdnat`,
   `/etc/sdnat/sdnat.json` and `<state_dir>/sdnatd.log`; environment variables use
   the `SDNAT_` prefix. Socket and lock file names remain `control.sock`,
   `control.sock.lock` and `control.sock.up.lock` inside the new run directory.
5. **No compatibility aliases.** There are no `vnet` symlinks, forwarding C
   symbols, duplicated paths or environment fallbacks. Pre-1.0 consumers are
   expected to update.
6. **Migration is explicit.** Stop any pre-rename daemon with the old
   `vnet down` before installing the new binaries. Journals and runtime state
   under `/run/vnet` are not copied automatically because they describe live
   host state that must be reconciled, not replayed under a new name.
   Pre-rename configs remain valid JSON but must be moved to the new default
   path or passed explicitly.
7. **Shard names.** The shard build graph (ADR-018) keeps the `sdnat-*`
   module names introduced by the prior rename; no new shard is added here.

## Consequences

- Product, binaries, C ABI, environment variables and default paths share one
  consistent identity.
- Any local scripts, configs or hosts linked against `vnet_*` must update.
- The historical ADR-017 and pre-rename changelog entries remain as the record
  of the vRouter→vNET transition.

## Alternatives

- **Keep vNET.** Broad and clean, but collides visually with common
  virtual-network abbreviations and undersells the NAT focus.
- **Use SDN.** Shorter, but collides with the industry term for
  software-defined networking and is too generic to own.
