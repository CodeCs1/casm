# CASM (Code-dev's Assembly language)
The compiled language that I wrote for making easier when writting assembly language.

The compiler just compiled in tokenizing file.

## Installation
#### This project uses cmake for building and installing this.
To start building, simplely running
```sh
cmake . && make
```
To run test, run:
```sh
make test
```
To install, run:
```
make install
```


## Syntax
### Move instruction
```
<register> <- <number/register/etc>
```
Same as: ```mov <register> <number>```
### Function creation
It's now easy to create a function.
With this:
```
main [
    // setup stack
    ax <- 0
    ax <- ds
    ax <- es
    ss <- ax
    sp <- :h7C00

    hlt
]
```
or with parameter:
```
test(a) [
    cx <- a
    inc cx
    ret
]
```
Example file can be found in 'test' folder

### loop instruction
```loop <?> => <?>```
