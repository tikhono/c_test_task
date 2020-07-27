# c_test_task

## 1 Install [NNG](https://github.com/nanomsg/nng)

Linux installation:
```
  $ git clone https://github.com/nanomsg/nng
  $ cd nng
  $ mkdir build
  $ cd build
  $ cmake -G Ninja ..
  $ ninja
  $ ninja test
  $ ninja install
```

## 2 Install [Lib Sodium](https://github.com/jedisct1/libsodium)

Linux installation:
```
  $ ./configure
  $ make && make check
  $ sudo make install
```
  
## 3 Build project
```
  $ cmake CmakeLists.txt -B build
  $ cd build
  $ make
```
  
## 4 Run server or create file
```
  $ ./server tcp://127.0.0.1:8000
  or
  $ touch filename.txt
  $ vim filename.txt
```
  
## 5 Run client to parse from file or server
```
  $ ./client filename
  or
  $ ./client tcp://127.0.0.1:8000
```
