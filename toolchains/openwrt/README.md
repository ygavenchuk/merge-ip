# A toolset to build the application for the [OpenWrt](https://openwrt.org/) platform

### Disclaimer(s)

First of all, this toolset is just a PoC. I prepared it for my own needs.

As you may guess from the [config](./MediaTek-MT7621-v21.02.7.config) file I use the 
`Asus RT AC65p` router to play with the `OpenWrt`. This device has not a regular 
`x86` CPU architecture, so it requires a cross-compilation. For that purpose I prepared
the `alpine`-based [docker image](./alpine.Dockerfile). Unfortunately, after building
all `OpenWrt`'s tools the final image size is `7.14GB`.

### How to use
There is a `Makefile` (I'm a bit more familiar with it than with `cmake`, so for the 
toolchain I decided to use it) with several targets:
 - `build_image` - obviously it's used to build the image;
 - `run_container` - runs container in the interactive mode, so it provides you access
                     to the development environment;
 - `app` - in fact this target is for internal usage, i.e. inside the container;
 - `build_app_in_container` - allows you to build the application from the host machine
    and puts the result into the `toolchain/openwrt/build/merge-ip` file.

### Build
So, you're supposed to do the next steps:
1. `cd toolchain/openwrt` (the `Makefile` uses relative paths, sorry);
2. `make build_image` (only once, until you don't have the image yet. Be patient, 
    it may take a while - about an hour or so, depends on your host machine, i-net 
    speed, etc.);
3. `make build_app_in_container`;
4. get the `merge-ip` binary file from the `toolchain/openwrt/build/` directory;
