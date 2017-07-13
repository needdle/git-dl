#!/bin/sh
log=$(which git-dl-log)
if [ "$log" != "/usr/local/bin/git-dl-log" ]; then
	if [ ! -f ../gitlog ]; then
		cd .. 
		make install
		cd -
	fi
fi
log=${log:=../git-dl-log}

testLog() 
{
	TMP_PB=`mktemp -t XXXXXXXX.pb` || exit 1
	git dl pb a 4 
	fast -d a.pb > a.txt
	#assertSame 634978e707a30ced35bb9083c6fbfb8489b71bb6bc6849629b23fc45cb8c8687 $(cat a.txt | shasum -a 256 | awk '{print $1}')
	rm -f $TMP_PB
}

if [ ! -f ~/mirror/github.com/kward/shunit2/source/2.1/src/shunit2 ]; then
	git clone https://github.com/kward/shunit2 ~/mirror/github.com/kward/shunit2
fi
. ~/mirror/github.com/kward/shunit2/source/2.1/src/shunit2
