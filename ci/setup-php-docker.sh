#!/bin/sh

set -eux

docker-php-ext-install sockets pcntl
docker-php-ext-enable --ini-name 0-sockets.ini sockets
docker-php-ext-enable pcntl
