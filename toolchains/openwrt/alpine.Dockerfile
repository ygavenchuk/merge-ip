#
# Copyright 2024 Yurii Havenchuk.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

FROM alpine:3.20.3

# install system packages
# @see https://openwrt.org/docs/guide-developer/toolchain/install-buildsystem
RUN apk add argp-standalone asciidoc bash bc binutils bzip2 cdrkit coreutils \
      diffutils elfutils-dev findutils flex musl-fts-dev ninja g++ gawk gcc \
      gettext git grep gzip intltool libxslt linux-headers make musl-libintl \
      musl-obstack-dev ncurses-dev openssl-dev patch perl py3-pip \
      py3-distutils-extra py3-virtualenv python3-dev rsync tar unzip util-linux \
      wget zlib-dev

# clone OpenWRT project
# @see https://openwrt.org/docs/guide-developer/toolchain/use-buildsystem
RUN mkdir -p /opt/openwrt \
    && cd /opt/openwrt \
    && git clone https://git.openwrt.org/openwrt/openwrt.git . \
    && git pull


# ===== Version specific commands =====

# optional checkout to the specific version
RUN cd /opt/openwrt && git checkout v21.02.7

COPY MediaTek-MT7621-v21.02.7.config /opt/

# ====== build toolchain ======

# ======== Update the feeds ========
RUN cd /opt/openwrt \
    && ./scripts/feeds update -a \
    && ./scripts/feeds install -a

# ======== download config for the specific target and build toolchain ========
# RUN wget https://downloads.openwrt.org/releases/21.02.7/targets/ramips/mt7621/config.buildinfo -O /opt/openwrt/.config \
RUN cp /opt/MediaTek-MT7621-v21.02.7.config /opt/openwrt/.config \
    && cd /opt/openwrt \
    && FORCE_UNSAFE_CONFIGURE=1 PKG_HASH=skip make toolchain/install

# set up environment variables (i.e. path to compiller, linker, libraries, etc)
ENV STAGING_DIR=/opt/openwrt/staging_dir/chaos_calmer/staging_dir

# you may need to use your own path uere
ENV TOOLCHAIN_DIR=$STAGING_DIR/toolchain-mipsel_24kc_gcc-8.4.0_musl
ENV LDCFLAGS=$TOOLCHAIN_DIR/usr/lib
ENV LD_LIBRARY_PATH=$TOOLCHAIN_DIR/usr/lib
ENV PATH=$TOOLCHAIN_DIR/bin:$PATH
