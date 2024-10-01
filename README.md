# Merge-IP

A simple CLI tool that merges 2 or more sequential CIDRs where it is possible.

I.e. assume you have next CIDRs:
 - `10.1.0.0/24`
 - `10.1.1.0/24`
 - `10.1.2.0/24`
 - `10.1.3.0/24`

It's redundant to have 4 routing records for those CIDRs, it would be more
convenient to combine them into, say `10.1.0.0/22`

As a side effect the tool removes duplicates and orders CIDRs in ascending order.
It reads CIDRs or IPs separated by space(s) or new line(s) from standard input or
file and prints results into the standard output.

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
