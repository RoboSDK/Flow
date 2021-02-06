#!/usr/bin/env bash

set -o nounset

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null && pwd)"
source "${DIR}/docker-options"

docker build \
    --build-arg DOCKER_UID="${DOCKER_UID}" \
    --build-arg DOCKER_GID="${DOCKER_GID}" \
    --build-arg DOCKER_USERNAME="${DOCKER_USERNAME}" \
    --build-arg VARIANT="${VARIANT}"\
    --build-arg TAG="${TAG}" \
    --build-arg CUSTOM_BUILD_COMMAND="${CUSTOM_BUILD_COMMAND}" \
    -t ${DOCKER_NAME}:${TAG} .
