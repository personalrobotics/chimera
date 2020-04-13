FROM ubuntu:18.04 as builder

# Install APT dependencies required for building chimera.
RUN apt-get update -qq \
  && apt-get install -y -qq \
  cmake \
  clang \
  clang-format \
  llvm-6.0-dev \
  llvm-6.0-tools \
  libboost-dev \
  libclang-6.0-dev \
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

#############################################################################
FROM ubuntu:18.04

# Install APT dependencies required for running chimera.
RUN apt-get update -qq \
  && apt-get install -y -qq \
  llvm-6.0-tools \
  libclang-6.0 \
  libedit2 \
  libyaml-cpp0.5v5 \
  zlib1g \
  && rm -rf /var/lib/apt/lists/*

# Install Chimera from the build image.
COPY --from=builder /usr/local/bin/chimera /usr/local/bin/chimera

# Create a non-root user to run Chimera application
RUN useradd --create-home -s /bin/bash user
WORKDIR /home/user
USER user

# Set chimera as the default entrypoint.
ENTRYPOINT ["/usr/local/bin/chimera"]
