# Git-Deep-Learning tool 

This utility project contains a [git](https://github.com/git/git) command to
turn the change history of Git repository into
[Protobuf](https://github.com/google/protobuf) structures for [Tensorflow
Fold](https://github.com/tensorflow/fold). 

If the Git diff hunks come from a source code file (determined by a file
extension recognisable by srcML), we turn them into [flattened abstract
syntax trees](https://github.com/yijunyu/fast) binary structures in Protobuf.

## Installation

First install the following prerequisites.

### Dependencies (prerequisites)
* [git](https://github.com/git/git)
* [srcML](http://www.srcml.org)
* [protobuf](https://github.com/google/protobuf)
* [Tensorflow Fold](https://github.com/tensorflow/fold)
* [fast](https://github.com/yijunyu/fast)
* [GNU Parallel](https://www.gnu.org/software/parallel)

Then perform the following commands to build the tool:
```
make
make install
```

## Usage

1. Display detailed log information of Git
```
git dl log
```

2. Turn the log information into a protobuf representation
```
git dl log | gitlog a
```
where `a.pb` contains the protobuf information in binary.

3. Show the binary protobuf in nested textual format
```
cat a.pb | protoc -I. --decode=fast.Log git.proto
```

4. Concatenate two protobuf binary files into a single one, removing the duplicates
```
catlog a-0.pb a-1.pb a.pb
cat a.pb | protoc -I. --decode=fast.Log git.proto
```
where a-0.pb and a-1.pb are logs, a.pb should be the merged (unique) logs.

5. Split the log equally into M / ((M+N-1)/N) jobs, where M is the number of logs, N is the number of splitted files
```
git dl log | gitlog a $N
```
where $N is the number of splitted files, in other words, the command will produce a series of files
```
a-0.pb, ..., a-($N-1).pb
```
so that merging them together using `catlog a-*.pb a.pb` is the same as the single result file `a.pb` of 
running the previous ```gitlog a``` command.

6. Parsing larger repositories it can take substantial amount of time. 
To speed it up, you can use [GNU Parallel](https://www.gnu.org/software/parallel) to run the process by the following commands:

```
git dl log | gitlog -p a $N
parallel --plus "cat {} | gitlog {/.log/}" ::: a-*.log
catlog a-*.pb a.pb
rm -f a-*.log a-*.pb
```
where `-p` tells the processor to split the log first into `a-*.log` then the next command produces `a-*.pb` in parallel.

The above command sequence has also been simplified to a single command below:
```
git dl pb a $N
```
