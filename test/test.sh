#!/bin/sh
log=$(which git-dl-log)
if [ "$log" != "/usr/local/bin/git-dl-log" ]; then
	if [ ! -f ../git-dl-log ]; then
		cd .. 
		make install
		cd -
	fi
fi
log=${fast:=../git-dl-log}

testLog() 
{
	TMP_PB=`mktemp -t XXXXXXXX.pb` || exit 1
	git dl log | gitlog $TMP_PB
	cat $TMP_PB | protoc -I.. ../git.proto --decode=fast.Log > a.txt
	assertSame 634978e707a30ced35bb9083c6fbfb8489b71bb6bc6849629b23fc45cb8c8687 $(cat $TMP_PB | protoc -I.. ../git.proto --decode=git.Log | shasum -a 256 | awk '{print $1}')
	rm -f $TMP_PB
}

if [ ! -f ~/mirror/github.com/kward/shunit2/source/2.1/src/shunit2 ]; then
	git clone https://github.com/kward/shunit2 ~/mirror/github.com/kward/shunit2
fi
. ~/mirror/github.com/kward/shunit2/source/2.1/src/shunit2
