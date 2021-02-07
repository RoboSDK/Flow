#!/usr/bin/env bash

set -o nounset

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null && pwd)"
source "${DIR}/../docker-options"
TAG="base"

docker build \
    --build-arg DOCKER_UID="${DOCKER_UID}" \
    --build-arg DOCKER_GID="${DOCKER_GID}" \
    --build-arg DOCKER_USERNAME="${DOCKER_USERNAME}" \
    -t ${VARIANT}:${TAG} .
