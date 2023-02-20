BIN = bin/main
CC = clang
CFLAGS = --std=c11 -fuse-ld=lld -rtlib=compiler-rt
IMAGES = /var/images/alpine /var/images/ubuntu

all: $(BIN)

images: $(IMAGES)

bin/main: main/main.c | bin
	$(CC) $(CFLAGS) $^ -o $@

bin:
	@mkdir bin

images/ubuntu.tar:
	@mkdir -p images
	@CONTAINER_ID=`docker run -d ubuntu`; \
	docker export $$CONTAINER_ID -o $@; \
	docker rm $$CONTAINER_ID

images/alpine.tar:
	@mkdir -p images
	@CONTAINER_ID=`docker run -d alpine`; \
	docker export $$CONTAINER_ID -o $@; \
	docker rm $$CONTAINER_ID

/var/images/alpine: images/alpine.tar
	@mkdir -p $@
	@tar -x -f $^ -C $@

/var/images/ubuntu: images/ubuntu.tar
	@mkdir -p $@
	@tar -x -f $^ -C $@

clean:
	@rm -rf $(BIN)
	@rm -rf $(IMAGES)
