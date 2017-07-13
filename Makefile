BIN_DIR   = /usr/local/bin
SHARE_DIR   = /usr/local/share
LOADER    += git-dl
LOADER    += gitlog
LOADER    += catlog
COMMANDS  += git-dl-log
COMMANDS  += git-dl-pb
COMMANDS  += git-dl-slice
target += src/gen/fast_pb2.py
target += gitlog
target += catlog

ifeq ($(UNAME_S),Darwin)
PB_LIB=$(shell pkg-config --libs protobuf)
SAX_LIB=osx/libsrcsax.a
XML2_INCLUDE=-I/usr/local/Cellar/libxml2/2.9.4_2/include/libxml2
else
PB_LIB=/usr/local/lib/libprotobuf.a
SAX_LIB=/usr/local/lib/libsrcsax.a
XML2_INCLUDE=-I/usr/include/libxml2
endif

all: $(target)

install: $(LOADER) $(COMMANDS)
	install -d -m 0755 $(BIN_DIR) $(SHARE_DIR)
	install -m 0755 $(LOADER) $(BIN_DIR)
	install -m 0644 $(COMMANDS) $(BIN_DIR)

uninstall:
	test -d $(BIN_DIR) && \
	cd $(BIN_DIR) && \
	rm -f $(LOADER) $(COMMANDS)

%: src/%.in
	cpp -I. -E -P $^ | grep -v "^0$$" > $@

src/gen/fast.pb.h src/gen/fast.pb.cc: src/fast.proto
	mkdir -p src/gen
	cd src && protoc --cpp_out=gen fast.proto

CCFLAGS=-g
CCFLAGS=-O3

gitlog: src/gen/fast.pb.cc src/gitlog.cc
	c++ $(CCFLAGS) -I. -I/usr/local/include -Isrc -Isrc/gen -DPB_fast $^ $(PB_LIB) -o $@

catlog: src/gen/fast.pb.cc src/catlog.cc
	c++ $(CCFLAGS) -I. -I/usr/local/include -Isrc -Isrc/gen -DPB_fast $^ $(PB_LIB) -o $@

clean:
	rm -rf $(target) src/gen temp.* test/temp.* *.dSYM

test::
	cd test; test.sh; cat a.txt

src/gen/fast_pb2.py: src/fast.proto
	mkdir -p src/gen
	cd src && protoc -I=. --python_out=gen fast.proto

a.pb:
	git dl pb a 1

src/fold.py: src/gen/fast_pb2.py

load: src/fold.py a.pb
	python $^
