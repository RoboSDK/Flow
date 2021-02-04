FROM manuelmeraz/flow:latest

USER root

ARG DOCKER_UID=1001
ENV DOCKER_UID ${DOCKER_UID}

ARG DOCKER_GID=1001
ENV DOCKER_GID ${DOCKER_GID}

ARG DOCKER_USERNAME="user"
ENV DOCKER_USERNAME ${DOCKER_USERNAME}
ENV USER ${DOCKER_USERNAME}

# Delete any custom user/group from the base image,
# as they likely have the wrong uid/gid.
RUN userdel ${DOCKER_USERNAME} | true
RUN groupdel ${DOCKER_USERNAME} | true
RUN userdel "user" | true
RUN groupdel "user" | true

# Add the user
RUN if [ -z "$(getent group  ${DOCKER_GID})" ] ; then addgroup --gid ${DOCKER_GID} ${DOCKER_USERNAME}; fi
RUN if [ -z "$(getent passwd ${DOCKER_UID})" ] ; then adduser --uid ${DOCKER_UID} --gid ${DOCKER_GID} --disabled-password --gecos '' ${DOCKER_USERNAME}; fi

# Turn off passwords for our user
RUN echo "$DOCKER_USERNAME ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers

ENV HOME /home/${DOCKER_USERNAME}

ARG CUSTOM_BUILD_COMMAND=""
ENV CUSTOM_BUILD_COMMAND ${CUSTOM_BUILD_COMMAND}
RUN test -z "${CUSTOM_BUILD_COMMAND}" || eval "${CUSTOM_BUILD_COMMAND}" || :
RUN echo ${CUSTOM_BUILD_COMMAND}

WORKDIR ${HOME}
USER ${DOCKER_USERNAME}

ENTRYPOINT /bin/sh -c "sudo service nginx start;while true; do sleep 10; done"
