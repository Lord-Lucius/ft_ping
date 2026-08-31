<div align="center">

# 🛰️ ft_ping

### *A from-scratch reimplementation of the classic* `ping` *utility in C*

![Language](https://img.shields.io/badge/language-C-blue?style=flat-square)
![Standard](https://img.shields.io/badge/standard-gnu11-informational?style=flat-square)
![Platform](https://img.shields.io/badge/platform-Linux-lightgrey?style=flat-square)


*Reference implementation:* `inetutils-2.0`

</div>

---

## 📖 About

`ft_ping` is a reimplementation of the Unix `ping` command, a network diagnostic
tool used to test reachability of a host on an IP network and measure the
**round-trip time** of packets sent to that host.

Under the hood, it crafts **ICMP Echo Request** packets, sends them through a
**raw socket**, and listens for the matching **ICMP Echo Reply** messages —
computing the elapsed time with microsecond precision.

This project was built strictly from scratch: no calls to the system `ping`,
no reuse of an existing implementation's source.

---

## ✨ Features

### Mandatory
- ✅ IPv4 and FQDN targets
- ✅ `-v` verbose mode (prints anomalous ICMP packets without exiting)
- ✅ `-?` usage screen
- ✅ Output formatting matches `inetutils-2.0`
- ✅ Clean statistics on `SIGINT` (packets sent/received, loss %, RTT min/avg/max/stddev)
- ✅ Robust error handling — no segfaults, no leaks

### Bonus *(planned / in progress)*
| Flag | Description |
|------|-------------|
| `-s` | Payload size |
| `-w` | Global deadline |
| `-W` | Per-packet timeout |
| `-n` | No DNS resolution |
| `-l` | Preload |
| `-f` | Flood mode |
| `-p` | Custom fill pattern |
| `--ttl` | Custom TTL |
| … | *more to come* |

---

## 🔧 Build

### Requirements
- A Linux environment (kernel **> 3.14**, Debian-like recommended)
- `gcc` or `clang`
- `make`

### Build targets

```bash
make           # Build the project
make re        # Full rebuild
make clean     # Remove object files
make fclean    # Remove objects + binary
make debug     # Build with -g3, AddressSanitizer & UBSan
make setcap    # Grant CAP_NET_RAW (run without sudo)
make help      # Show available targets
```

The build system auto-detects every `.c` file under `src/` (recursively) and
uses `-MMD -MP` to regenerate object files whenever a header changes.

---

## 🚀 Usage

`ping` requires raw-socket privileges. Pick one of:

```bash
# Option 1 — run as root
sudo ./ft_ping google.com

# Option 2 — one-time capability grant, then run as regular user
make setcap
./ft_ping google.com
```

### Basic examples

```bash
./ft_ping 8.8.8.8
./ft_ping google.com
./ft_ping -v example.org
./ft_ping -?
```

### Sample output

```
PING google.com (142.250.74.238): 56 data bytes
64 bytes from 142.250.74.238: icmp_seq=0 ttl=116 time=12.345 ms
64 bytes from 142.250.74.238: icmp_seq=1 ttl=116 time=11.987 ms
64 bytes from 142.250.74.238: icmp_seq=2 ttl=116 time=12.104 ms
^C
--- google.com ping statistics ---
3 packets transmitted, 3 packets received, 0% packet loss
round-trip min/avg/max/stddev = 11.987/12.145/12.345/0.148 ms
```

---

## 🗂️ Project structure

```
ft_ping/
├── Makefile
├── README.md
├── include/
│   └── ft_ping.h        # Public headers
├── src/
│   ├── main.c           # Entry point, argument parsing
│   ├── socket/          # Raw socket setup
│   ├── packet/          # ICMP packet crafting, checksum
│   ├── loop/            # Send/receive loop, signal handling
│   ├── stats/           # RTT statistics, final summary
│   └── utils/           # Misc helpers (error, dns, time)
└── build/               # Auto-generated object & dep files
```

*(Structure indicative — to be adapted as the project grows.)*

---

## 🧠 Technical notes

- **Raw sockets** — created with `socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)`.
  This bypasses the kernel's transport layer and requires elevated privileges.
- **ICMP checksum** — standard 16-bit one's-complement sum, computed over the
  ICMP header + payload only (the kernel fills in the IP header).
- **Packet identification** — we tag outgoing packets with
  `getpid() & 0xFFFF` to filter incoming replies destined to us.
- **Timing** — high-resolution timestamps via `clock_gettime(CLOCK_MONOTONIC, …)`
  to avoid artifacts from wall-clock adjustments (NTP, DST).
- **Signals** — `SIGINT` triggers graceful shutdown + final statistics;
  `SIGALRM` (via `setitimer`) paces packet emission at 1 Hz.
- **No reverse DNS** on received packets — the target is resolved once at
  startup via `getaddrinfo`, then the raw IP is displayed for replies.

---

## ⚠️ Constraints (from the subject)

- 🚫 Calling the system `ping` (directly or via `fork`/`exec`) is forbidden.
- 🚫 Reusing the source code of an existing `ping` implementation is forbidden.
- ✅ The entire libc is allowed.
- ✅ The `printf` family is allowed (no need to reimplement it).
- ✅ Reception tolerance of **±30 ms** is accepted.

---

## 📚 References

- [RFC 792 — Internet Control Message Protocol](https://www.rfc-editor.org/rfc/rfc792)
- [RFC 1122 — Requirements for Internet Hosts](https://www.rfc-editor.org/rfc/rfc1122)
- [`inetutils-2.0` source](https://www.gnu.org/software/inetutils/) — reference implementation
- `man 7 raw`, `man 7 icmp`, `man 2 socket`, `man 3 getaddrinfo`

---

<div align="center">

*Built with care.*

</div>
