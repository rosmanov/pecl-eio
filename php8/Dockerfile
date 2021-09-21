FROM php:8.1.0RC2-cli

RUN mkdir -p /usr/src/pecl-eio
WORKDIR /usr/src/pecl-eio

COPY . .

RUN sh build.sh

ENV NO_INTERACTION=1
CMD sh run-tests.sh

# vim: ft=dockerfile
