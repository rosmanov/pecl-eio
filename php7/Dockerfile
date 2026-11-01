FROM php:7.4.23-cli

ARG SANITIZER=""
ENV SANITIZER="${SANITIZER}"

WORKDIR /usr/src/pecl-eio
COPY . .

RUN ./ci/setup-php-docker.sh
RUN ./ci/build.sh
RUN docker-php-ext-enable --ini-name z-eio.ini eio

ENV NO_INTERACTION=1
CMD ["./ci/test.sh"]

# vim: ft=dockerfile
