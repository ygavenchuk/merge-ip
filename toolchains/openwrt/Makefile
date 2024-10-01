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

.PHONY: container

build_image:
	docker build -f alpine.Dockerfile -t openwrt:v21.02.07


run_container:
	docker run \
		-v "$(shell pwd)/../../":/opt/package/merge-ip:ro \
		-v "$(shell pwd)/build/":/opt/build:rw \
		--rm -i -t \
		--name openwrt \
		openwrt:v21.02.7 \
		/bin/bash

app:
	/opt/openwrt/staging_dir/host/bin/cmake \
            -DCMAKE_BUILD_TYPE=Release \
            -DCMAKE_MAKE_PROGRAM=ninja \
            -DCMAKE_C_COMPILER=mipsel-openwrt-linux-gcc \
            -G Ninja \
            -S /opt/package/merge-ip/ \
            -B /opt/build \
	&& /opt/openwrt/staging_dir/host/bin/cmake \
            --build /opt/build/ \
            --target merge-ip

build_app_in_container:
		docker run \
    		-v "$(shell pwd)/../../":/opt/package/merge-ip:ro \
    		-v "$(shell pwd)/build/":/opt/build:rw \
    		--rm \
    		--name openwrt \
    		openwrt:v21.02.7 \
    		make -f /opt/package/merge-ip/toolchains/openwrt/Makefile app
