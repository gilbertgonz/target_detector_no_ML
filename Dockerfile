FROM ubuntu:jammy

ENV DEBIAN_FRONTEND=noninteractive

RUN apt update && apt install -y \
    build-essential cmake libopencv-dev gdb x11-apps \
    # the packages below are just to minimize GTK and accessibility warnings
    at-spi2-core libcanberra-gtk-module libcanberra-gtk3-module

COPY main.cpp /main.cpp
COPY CMakeLists.txt /CMakeLists.txt
COPY assets/ /assets/

RUN mkdir build && cd build \
    && cmake .. && make

# Clean up
RUN apt remove -y build-essential cmake

CMD [ "./build/main" ]