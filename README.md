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
  
## 2 Build project
```
  $ cmake CmakeLists.txt -B build
  $ cd build
  $ make
```
  
## 3 Run server
```
  $ ./server tcp://127.0.0.1:8000
```
  
## 4 Run client
```
  $ ./client filename
  or
  $ ./client tcp://127.0.0.1:8000
```
