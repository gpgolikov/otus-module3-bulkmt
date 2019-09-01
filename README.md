# otus-module2-bulk

OTUS modile 2 homework - _bulk_

## installation
```
$ curl -sSL "https://bintray.com/user/downloadSubjectPublicKey?username=bintray" | apt-key add -
$ echo "deb http://dl.bintray.com/gpgolikov/otus-cpp xenial main" | tee -a /etc/apt/sources.list.d/otus.list

$ apt update
$ apt install bulk
```

## installation of libc++1-7
```
$ curl -sSL "https://build.travis-ci.org/files/gpg/llvm-toolchain-xenial-7.asc" | apt-key add -
$ echo "deb http://apt.llvm.org/xenial/ llvm-toolchain-xenial-7 main" | tee -a /etc/apt/sources.list >/dev/null

$ apt update
$ apt install libc++1-7 libc++abi-7

```
