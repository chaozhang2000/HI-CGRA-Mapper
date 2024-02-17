# README

## Quick start

Ubuntu22.04

1. install dependency
```
sudo apt-get install clang-12 make build-essential llvm llvm-12-dev graphviz bison flex
```

2. add MAPPER\_HOME to ~/.bashrc  
	1. vim ~/.bashrc then add at the last line,for example "export MAPPER\_HOME=/home/user/HIT-CGRA-Mapper"
	2. source ~/.bashrc

3. config HIT-CGRA-Mapper and compile
```
make menuconfig
make
```
