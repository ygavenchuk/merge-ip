# Merge-IP
[![Build Status](https://img.shields.io/github/actions/workflow/status/ygavenchuk/merge-ip/ci.yml?style=plastic&logo=github&label=Build%20and%20Tests
)](https://img.shields.io/github/actions/workflow/status/ygavenchuk/merge-ip/ci.yml?style=plastic&logo=github&label=Build%20and%20Tests
)
[![License](https://img.shields.io/badge/license-Apache2.0-green.svg?style=plastic)](LICENSE)

A simple CLI tool that merges 2 or more sequential CIDRs where it is possible.

I.e. assume you have next CIDRs:
 - `10.1.0.0/24`
 - `10.1.1.0/24`
 - `10.1.2.0/24`
 - `10.1.3.0/24`

It's redundant to have 4 routing records for those CIDRs, it would be more
convenient to combine them into, say `10.1.0.0/22`

As a side effect, the tool removes duplicates and orders CIDRs in ascending order.
It reads CIDRs or IPs separated by space(s) or new line(s) from a standard input or
file and prints results into the standard output.

## Supported OS
 - [![Ubuntu latest](https://img.shields.io/badge/Ubuntu-24.04%20LTS-green.svg?style=plastic&logo=ubuntu)](https://ubuntu.com/)
 - [![MacOS 13 latest](https://img.shields.io/badge/MacOS-13%20Ventura,%2014%20Sonoma-green.svg?style=plastic&logo=apple)](https://www.apple.com/macos/)
 - [![Windows latest](https://img.shields.io/badge/Windows-11-green.svg?style=plastic)](https://www.microsoft.com/en-us/windows)
 - [![OpenWrt latest](https://img.shields.io/badge/OpenWrt-21.02,%2022.03-green.svg?style=plastic&logo=openwrt)](https://openwrt.org/)

## Supported compilers
 - [![GCC latest](https://img.shields.io/badge/GCC-13.2-green.svg?style=plastic&logo=gcc)](https://gcc.gnu.org/)
 - [![Clang latest](https://img.shields.io/badge/Clang-17.0-green.svg?style=plastic&logo=clang)](https://clang.llvm.org/)

## Usage
 - `./merge-ip -f file-with-cidrs.txt`
 - `cat file-with-cidrs.txt | merge-ip`

## Build

```bash
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_MAKE_PROGRAM=ninja -G Ninja
cmake --build --target merge-ip
```

### Build for OpenWrt
See instructions [here](./toolchains/openwrt/README.md)



#### Special thanks
This tool is inspired by:
 - https://github.com/zhanhb/cidr-merger;
 - https://docs.python.org/3/library/ipaddress.html#ipaddress.IPv4Network;
