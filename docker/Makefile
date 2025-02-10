GIT_DIR=../..

avswu: Dockerfile polkadot-grpc-dev
	cd ${GIT_DIR}/avswu/docker; docker build . -t $@

polkadot-grpc-dev: ${GIT_DIR}/polkadot-grpc-dev/docker/Dockerfile  polkadot-dev
	cd ${GIT_DIR}/$@/docker; docker build . -t $@

polkadot-dev: ${GIT_DIR}/polkadot-dev/docker/Dockerfile rust-dev
	cd ${GIT_DIR}/$@/docker; docker build . -t $@

rust-dev: ${GIT_DIR}/rust-dev/docker/Dockerfile ubuntu-dev
	cd ${GIT_DIR}/$@/docker; docker build . -t $@

ubuntu-dev: ${GIT_DIR}/ubuntu-dev/docker/Dockerfile
	cd ${GIT_DIR}/$@/docker; docker build . -t $@

clean:
	docker image rm ${TARGET}