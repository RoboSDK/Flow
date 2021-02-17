#! /bin/bash

[[ -d /tmp/flow-docker ]] && rm -rf /tmp/flow-docker
git clone https://github.com/manuelmeraz/flow-docker /tmp/flow-docker

/tmp/flow-docker/install_scripts/install_libraries.sh

rm -rf /tmp/flow-docker
