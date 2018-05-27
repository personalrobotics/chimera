FROM ubuntu:16.04

# Install APT dependencies
RUN apt-get update -qq \
  && apt-get install -y -qq \
  cmake \
  clang \
  llvm-3.7-dev \
  llvm-3.7-tools \
  libboost-dev \
  libclang-3.7-dev \
  libedit-dev \
  libyaml-cpp-dev \
  libz-dev \
  && rm -rf /var/lib/apt/lists/*

# Compile and install chimera
COPY . /opt/chimera
WORKDIR /opt/chimera
RUN mkdir build && cd build \
  && cmake -DBUILD_TESTING=off .. \
  && make install

# Create a non-root user to run Chimera application
RUN useradd --create-home -s /bin/bash user
WORKDIR /home/user
USER user

# Set chimera as the default executable.
ENTRYPOINT ["/usr/local/bin/chimera"]