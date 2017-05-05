BIN_DIR   = /usr/local/bin
LOADER    += git-dl
LOADER    += gitlog
COMMANDS  += git-dl-log
target += git.pb.cc
target += git.pb.h
target += gitlog
target += git.proto
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
target += fast.proto

PB_LIB=$(shell pkg-config --libs protobuf)

all: $(target)

install:
	install -d -m 0755 $(BIN_DIR)
	install -m 0755 $(LOADER) $(BIN_DIR)
	install -m 0644 $(COMMANDS) $(BIN_DIR)

uninstall:
	test -d $(BIN_DIR) && \
	cd $(BIN_DIR) && \
	rm -f $(LOADER) $(COMMANDS)

%: src/%.in
	cpp -I. -E -P $^ | grep -v "^0$$" > $@

git.pb.h git.pb.cc: git.proto
	protoc --cpp_out=. $^

src/Literal.proto.in: LiteralType.proto
src/Unit.proto.in: LanguageType.proto
src/commit.proto.in: diff.proto
src/diff.proto.in: hunk.proto
src/fast.proto.in: ElementType.proto
src/fast.proto.in: Literal.proto
src/fast.proto.in: Unit.proto
src/git.proto.in: author.proto
src/git.proto.in: commit.proto
src/git.proto.in: fast.proto
src/hunk.proto.in: modline.proto

gitlog: git.pb.cc src/gitlog.cc src/fast.cc
	c++ -I. -Irapidxml -DPB_fast $^ $(PB_LIB) -o $@
#	c++ -g -I. -Irapidxml -DPB_fast $^ $(PB_LIB) -o $@

clean:
	rm -rf $(target) temp.* test/temp.* *.dSYM

test::
	cd test; test.sh; cat a.txt
