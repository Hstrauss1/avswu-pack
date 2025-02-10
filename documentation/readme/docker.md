# how to build and run the avswu docker ubuntu container

This ubuntu docker container is suitable to run a polkadot parachain

### run docker desktop app (on windows) or ssh into a avswu container

```bash
.\container-start.ps1
.\container-exec.ps1
```

> ssh -p 2222 root@127.0.0.1

## to build the ubuntu docker container image.

You should only need to do this once. This will take a long time (minutes) to run.

1. Open a command prompt in windows and go to the GitHub directory and cd into the 'docker' directory.

2. > cd docker
   >
   > docker build . -t avswu

**to build and save output to a log file**

> docker build --progress=plain . -t avswu 2>&1 > docker-build.log

to tail (display) the log file, in windows powershell, as it builds

> Get-Content .\docker-build.log -Wait -Tail 30

3. > docker image ls
   > to see your build

## run docker container, exposing ports, and mount the git directories from /root

When the 'docker run' command runs, the container starts and executes a startup .sh script that will set up the environment for development.

The startup script will ask for a passsword for vnc access, and complete the installation. Once completed, it should leave you at the root prompt in ubuntu.

This command will also take minutes to run. But, once it runs, your docker container is up and running and can also be stop or restarted quickly. You should only have to run the docker build and this docker run command _once_.

This command also asks for a password. Enter any password. This is simply the password for a vnc session into the container.

> docker run -it --privileged -p 2222:22 -p 5951:5901 -p 8001:8001 -p 9944:9944 -p 9615:9615 --mount type=bind,source=C:\Users\YOURDIRECTORY\GitHub\avswu,destination=~/avswu --hostname avswu1 --name avswu avswu

You can run docker container by using the run_container.ps1 script

> container-run.ps1

To execute a container shell (ssh into a shell)

> container-exec.ps1

## connect to a running docker container

Issue a docker exec command to open another or additional shells in to a container.

> docker start polka
>
> docker exec --privileged -it avswu /bin/bash

## errors

While starting the container, if you get an error that says,

> /root/startup/ubuntu-start.sh: not found

Run dos2unix on ubuntu-start.sh then run it

> dos2unix /root/startup/ubuntu-start.sh
>
> /root/startup/ubuntu-start.sh

To fix permission on linux directors, linux permissions are ordered: user group owner (ugo)

> chmod go-w avswu-server/

## starting more shells/windows in to the container

Once the container is running, you can docker exec, ssh, or vnc into it (using tinyvnc). The simplest thing to do is to docker exec into the container.

## connect to a running docker container

> docker exec -it avswu /bin/bash
