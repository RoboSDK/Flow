FROM ubuntu:20.04

# Disable Prompt During Packages Installation
ARG DEBIAN_FRONTEND=noninteractive

# Setup environment
USER root
ENV LANG C.UTF-8
ENV LC_ALL C.UTF-8

# Check to ensure that the container does not already contain
# the UID or GID (a common issue when using custom docker images).
ARG DOCKER_UID ${DOCKER_UID}
ARG DOCKER_GID ${DOCKER_GID}
ARG DOCKER_USERNAME ${DOCKER_USERNAME} 

ARG addgroup --gid ${DOCKER_GID} ${DOCKER_USERNAME}
ARG adduser --uid ${DOCKER_UID} --gid ${DOCKER_GID} --disabled-password --gecos '' ${DOCKER_USERNAME}

# Turn off passwords for our user
RUN echo "$DOCKER_USERNAME ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers

################################# Start ##############################
RUN apt update
RUN apt install -y "vim" "tmux" "silversearcher-ag" "git" "htop" \
		  "tree" "openssh-server" "openssh-client" \
		  "geany" "bash-completion" "cmake" "gcc-10" "g++-10" \
		  "python3-dev" "python-dev" "python3-pip" \
		  "build-essential" "clang" "clang-tidy" "clang-format" \
		  "clang-tools" "gdb" 

RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-10 100 --slave /usr/bin/g++ g++ /usr/bin/g++-10 --slave /usr/bin/gcov gcov /usr/bin/gcov-10

# install conan
RUN pip3 install conan

WORKDIR /tmp
RUN git clone https://github.com/Garcia6l20/cppcoro.git
WORKDIR /tmp/cppcoro/
RUN mkdir build
WORKDIR /tmp/cppcoro/build
RUN cmake -DCMAKE_BUILD_TYPE=Release ..
RUN make -j

RUN cp ./lib/libcppcoro.a /usr/local/lib
RUN cp -r ../include/* /usr/local/include

WORKDIR /tmp
RUN git clone https://github.com/axboe/liburing.git
WORKDIR /tmp/liburing
RUN ./configure
RUN make -j
RUN make install

WORKDIR /opt
RUN git clone https://github.com/ManuelMeraz/Flow.git
WORKDIR /opt/Flow
RUN mkdir build
WORKDIR /opt/Flow/build
RUN cmake -DENABLE_TESTING=ON -DENABLE_EXAMPLES=ON -DCMAKE_BUILD_TYPE=Release ..
RUN make -j 
RUN ctest -j
RUN make install 

WORKDIR /opt
RUN git clone https://github.com/ManuelMeraz/FlowExample.git
WORKDIR /opt/FlowExample
RUN mkdir build
WORKDIR /opt/FlowExample/build
RUN cmake --DCMAKE_BUILD_TYPE=Release ..
RUN make -j 

################################# End ##############################

ENV HOME /home/${DOCKER_USERNAME}
ENV USER ${DOCKER_USERNAME} 

