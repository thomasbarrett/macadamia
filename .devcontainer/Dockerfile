FROM alpine:latest

RUN apk update
RUN apk add clang lld compiler-rt musl-dev
RUN apk add git openssh make docker

# In order to test that we are using capabilities correctly, we
# want to run as an unprivledged `alpine` user by default, but allow
# the `alpine` user to easily escalate their privledges with `sudo`.
RUN apk add sudo
RUN echo '%wheel ALL = (ALL) NOPASSWD: ALL' > /etc/sudoers.d/wheel 
RUN adduser -D alpine
RUN adduser alpine wheel
USER alpine
