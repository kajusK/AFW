CMD=docker run -v $(shell pwd)/../:/home/dev/project -w /home/dev/project/tests --user $(shell id -u):$(shell id -g) afw_ceedling
IMAGE_NAME=afw_ceedling

tests: docker
	$(CMD) ceedling test:all

coverage: docker
	$(CMD) ceedling gcov:all

docker:
	@if [ -z "$$(docker images -q $(IMAGE_NAME))" ]; then \
		touch Dockerfile; \
		$(MAKE) docker_build; \
	fi

docker_build: Dockerfile
	docker build -t $(IMAGE_NAME) .

clean:
	@rm -rf ./build

.PHONY: docker clean coverage docker docker_build
