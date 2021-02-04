#!/usr/bin/env bash

# read the default options
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"
source "${DIR}/docker-options"

docker exec -it ${DOCKER_NAME} bash 
