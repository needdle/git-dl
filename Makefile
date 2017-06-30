BIN_DIR   = /usr/local/bin
SHARE_DIR   = /usr/local/share
LOADER    += git-dl
LOADER    += gitlog
LOADER    += catlog
COMMANDS  += git-dl-log
COMMANDS  += git-dl-pb
COMMANDS  += git-dl-slice
SCHEMA  += fast.proto
target += fast.pb.cc
target += fast.pb.h
target += gitlog
target += catlog
target += fast.proto
target += commit.proto
target += author.proto
target += diff.proto
target += modline.proto
target += hunk.proto
target += ElementType.proto
target += LanguageType.proto
target += Literal.proto
target += LiteralType.proto
target += Unit.proto
target += log.proto

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
	install -m 0644 $(SCHEMA) $(SHARE_DIR)

uninstall:
	test -d $(BIN_DIR) && \
	cd $(BIN_DIR) && \
	rm -f $(LOADER) $(COMMANDS)

%: src/%.in
	cpp -I. -E -P $^ | grep -v "^0$$" > $@

fast.pb.h fast.pb.cc: fast.proto
	protoc --cpp_out=. $^

src/Literal.proto.in: LiteralType.proto
src/Unit.proto.in: LanguageType.proto
src/commit.proto.in: diff.proto
src/diff.proto.in: hunk.proto
src/fast.proto.in: ElementType.proto
src/fast.proto.in: Literal.proto
src/fast.proto.in: Unit.proto
<<<<<<< HEAD
src/fast.proto.in: log.proto
src/log.proto.in: commit.proto
src/log.proto.in: author.proto
src/fast.proto.in: SmaliType.proto
=======
src/fast.proto.in: SmaliType.proto
src/git.proto.in: author.proto
src/git.proto.in: commit.proto
src/git.proto.in: fast.proto
>>>>>>> d24737609feab0510ddb65c0472fc65204e3225a
src/hunk.proto.in: modline.proto

CCFLAGS=-g
CCFLAGS=-O3

gitlog: fast.pb.cc src/gitlog.cc
	c++ $(CCFLAGS) -I. -I/usr/local/include -Isrc -DPB_fast $^ $(PB_LIB) -o $@

catlog: fast.pb.cc src/catlog.cc
	c++ $(CCFLAGS) -I. -I/usr/local/include -Isrc -DPB_fast $^ $(PB_LIB) -o $@

clean:
	rm -rf $(target) temp.* test/temp.* *.dSYM

test::
	cd test; test.sh; cat a.txt

fast_pb2.py: fast.proto
	protoc -I=. --python_out=. fast.proto

a.pb:
	git dl pb a 1

src/fold.py: fast_pb2.py

load: src/fold.py a.pb
	python $^
