# mytool

File checksum & entropy analyzer, split like a real distro package:

| Binary package  | Contains                                                        |
|-----------------|-----------------------------------------------------------------|
| `libmytool1`    | `libmytool.so.1.0.0` + SONAME symlink (runtime), `.symbols`     |
| `libmytool-dev` | header, `libmytool.so`, pkg-config + CMake package config       |
| `mytool`        | `/usr/bin/mytool`, man page, systemd service + timer, manifest  |

    gbp buildpackage --git-ignore-branch --git-builder="dpkg-buildpackage -us -uc -sa"
    lintian --pedantic --fail-on error,warning,info,pedantic ../*.changes
    autopkgtest ../*.deb . -- null
