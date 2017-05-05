= Git-Deep-Learning tool 

The project contains a git command to turn the change history of git repository
into protobuf structures for Tensorflow Fold.

== Usage

1. Display detailed log information of Git
```
git dl log
```

2. Turn the log information into a protobuf representation
```
git dl log | gitlog a.pb
```
where `a.pb` contains the protobuf information in binary.

3. Show the binary protobuf in nested textual format
```
cat a.pb | protoc -I. --decode=fast.Log git.proto
```
