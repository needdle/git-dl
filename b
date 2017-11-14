docker build -t fasttool/fast:git .
docker tag fasttool/fast:git fasttool/fast:git 
docker push fasttool/fast:git
docker run fasttool/fast:git
