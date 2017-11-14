FROM yijun/fast
RUN apk add -U --no-cache git make g++ protobuf-dev parallel
ADD . /git-dl
RUN cd /git-dl \
 && make \
 && make install
WORKDIR /git-dl
CMD ["sh", "-c", "git dl $@"]
