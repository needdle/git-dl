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
git dl log | gitlog a 1
```
where `a.pb` contains the protobuf information in binary.

3. Show the binary protobuf in nested textual format
```
cat a.pb | protoc -I. --decode=fast.Log git.proto
```

4. Concatenate two protobuf binary files into a single one, removing the duplicates
```
catlog t1.pb t2.pb tt.pb
cat tt.pb | protoc -I. --decode=fast.Log git.proto
```
where t1.pb and t2.pb are logs, tt.pb should be the merged (unique) logs.

5. Split the log equally into M / ((M+N-1)/N) jobs, where M is the number of logs, N is the number of splitted files
```
git dl log | gitlog t $N
```
where $N is the number of splitted files, in other words, the command will produce a series of files
```
t-0.pb, ..., t-($N-1).pb
```
so that merging them together using `catlog t-*.pb t0.pb` is the same as the result file `t-0.pb` of running the ```gitlog t 1``` command.

