# Copyright 2017 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# Defines a docker image that can build cras fuzzers.
#
FROM gcr.io/oss-fuzz-base/base-builder
LABEL maintainer="dgreid@chromium.org"

RUN apt-get -y update && \
      apt-get install -y \
      automake \
      build-essential \
      cargo \
      cmake \
      g++ \
      gdb \
      git \
      ladspa-sdk \
      libasound-dev \
      libdbus-1-dev \
      libgtest-dev \
      libncurses5-dev \
      libsbc-dev \
      libsndfile-dev \
      libspeexdsp-dev \
      libtool \
      libudev-dev \
      wget \
      zip
RUN apt-get clean
RUN cd /tmp && git clone https://github.com/ndevilla/iniparser.git && \
      cd iniparser && \
      make && \
      cp libiniparser.* /usr/local/lib && \
      cp src/dictionary.h src/iniparser.h /usr/local/include && \
      chmod 644 /usr/local/include/dictionary.h /usr/local/include/iniparser.h && \
      chmod 644 /usr/local/lib/libiniparser.a && \
      chmod 755 /usr/local/lib/libiniparser.so.*
RUN cd /usr/src/gtest && \
      cmake . && \
      make && \
      chmod 644 *.a && \
      cp *.a /usr/local/lib

# Need to build and install alsa so there is a static lib.
RUN mkdir -p /tmp/alsa-build && cd /tmp/alsa-build && \
      wget ftp://ftp.alsa-project.org/pub/lib/alsa-lib-1.1.4.1.tar.bz2 && \
      bzip2 -f -d alsa-lib-* && \
      tar xf alsa-lib-* && \
      cd alsa-lib-* && \
      ./configure --enable-static --disable-shared && \
      make clean && \
      make -j$(nproc) all && \
      make install

COPY . "${SRC}/adhd"
COPY cras/src/fuzz/build.sh "${SRC}/build.sh"
RUN mkdir -p /etc/cras && cp "${SRC}/adhd/cras-config/dsp.ini.sample" /etc/cras
