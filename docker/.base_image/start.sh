#!/usr/bin/env bash

# read the default options
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"
source "${DIR}/../docker-options"
TAG="base"

echo "Cleaning up existing ${DOCKER_NAME} containers..."
docker stop ${DOCKER_NAME} || true
docker rm   ${DOCKER_NAME} || true

echo "Running ${DOCKER_NAME}..."
docker run --privileged -d \
    -e "DISPLAY=unix$DISPLAY" -v "/tmp/.X11-unix:/tmp/.X11-unix:rw"  -v "/tmp:/tmp:rw" -v "/dev:/dev:rw" \
    -h ${DOCKER_NAME} \
    -v "$HOME:$HOME:rw" \
    --ipc=host --network host \
    --env DOCKER_NAME=${DOCKER_NAME} \
    --name ${DOCKER_NAME} -it ${VARIANT}:${TAG}

docker exec -u root ${DOCKER_NAME} sh -c "echo 127.0.0.1 ${DOCKER_NAME} >> /etc/hosts"
./bash.sh
