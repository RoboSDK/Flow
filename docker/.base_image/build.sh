#!/usr/bin/env bash

DOCKER_NAME='manuelmeraz/flow:base'

docker build \
  $@ \
  --network=host \
  -t ${DOCKER_NAME} .
