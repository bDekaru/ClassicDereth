# Set up a builder container that will actually compile GDLEnhanced
FROM alpine:latest AS builder

ENV \
  HOME=/home/gdle \
  USER=gdle \
  UID=1000

RUN apk add --no-cache \
    build-base \
    cmake \
    mariadb-connector-c-dev

RUN adduser -D -h $HOME -u $UID $USER

USER $USER
WORKDIR $HOME

ADD . .

RUN mkdir build && cd build \
  && rm -rf CMake* cmake* \
  && cmake -DCMAKE_BUILD_TYPE=Debug .. \
  && make clean \
  && make --jobs "$(nproc --all)"


# Set up the runtime image
FROM alpine:latest

ENV \
  HOME=/home/gdle \
  USER=gdle \
  UID=1000

RUN apk add --no-cache \
    libstdc++ \
    mariadb-connector-c

RUN adduser -D -h $HOME -u $UID $USER

COPY --from=builder --chown=1000 $HOME/build/GDLEnhanced $HOME/

USER $USER

WORKDIR /gdle

VOLUME [ "/gdle" ]

ENTRYPOINT [ "/home/gdle/GDLEnhanced" ]
CMD [ "--config", "server.cfg" ]

# if mysql is only available from localhost, mount the following
#/var/run/mysqld   /var/run/mysqld

# /gdle mount should mimic the Bin folder and contain configs
# and Data folder
