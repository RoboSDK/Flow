A docker image is available to start playing around with this library without installing it onto your machine. 


```
./build.sh # Pulls the docker image and builds a local docker image
./start.sh # Starts the docker container and attaches to it
```

If you leave bash session you may reattach with:

```
./bash.sh # attaches to the container again if previously disconnected
```

At the moment the default user is the root user.

TODO: Create a user matching the user name in the Dockerfile

In the `/opt` directory Flow will be cloned and already built. There is
also a minimal project that links to Flow called FlowExample.
