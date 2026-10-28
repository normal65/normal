# Component patches

Patches under this directory must be re-applied after `idf.py reconfigure` /
`idf.py fullclean` if the corresponding managed component is re-fetched.

## Apply

From the repo root (Git Bash / MSYS):

```bash
# NimBLE: refuse host_task when nimble_port_init fails
patch -p1 -d managed_components/h2zero__esp-nimble-cpp \
  < patches/h2zero__esp-nimble-cpp_check_nimble_port_init.patch

# micro-mp3: initialize slen
patch -p1 -d managed_components/esphome__micro-mp3-new \
  < patches/esphome__micro-mp3-new_slen-init.patch
```

On Windows PowerShell, prefer `git apply` from the managed component directory, or
Git Bash `patch` as above.

## Files

| Patch | Component | Why |
|-------|-----------|-----|
| `h2zero__esp-nimble-cpp_check_nimble_port_init.patch` | `h2zero/esp-nimble-cpp` | Avoid null eventq crash after HCI init failure |
| `esphome__micro-mp3-new_slen-init.patch` | `esphome/micro-mp3-new` | Initialize `slen` before use |
