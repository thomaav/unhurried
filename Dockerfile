FROM ubuntu:22.04

SHELL ["/bin/bash", "-c"]
RUN apt update && apt install -y gcc make build-essential cmake clang pkg-config git
RUN apt install -y libasound2-dev libx11-dev libxrandr-dev libxi-dev libgl1-mesa-dev libglu1-mesa-dev libxcursor-dev libxinerama-dev libwayland-dev libxkbcommon-dev

RUN git clone https://github.com/emscripten-core/emsdk.git
WORKDIR emsdk/
RUN ./emsdk install latest
RUN ./emsdk activate latest

COPY . /app

WORKDIR /app
RUN git submodule init && git submodule update
RUN mkdir -p build
WORKDIR /app/build
RUN source /emsdk/emsdk_env.sh && emcmake cmake .. -DPLATFORM=Web
RUN make
WORKDIR /app/build/bin

ENTRYPOINT ["python3", "-m", "http.server"]
