# runs the container
# map port 22 for ssh
# map port 5901 for vnc
# map port 8001 for localhost front end (when running it from the container)
# map port 9944 for wasm websocket for polkadot
# map port 4101 for avswu-server
# map port 58851 for grpc calls from veins VirtualBox
docker run -it --privileged `
    --mount "type=bind,source=C:\Users\jerom\Documents\GitHub\avswu,destination=/avswu" `
    --publish 2222:22 `
    --publish 5951:5901 `
    --publish 8001:8001 `
    --publish 9944:9944 `
    --publish 9615:9615 `
    --publish 4101:4101 `
    --publish 58851:50051 `
    --hostname avswu1 `
    --name avswu `
    avswu
