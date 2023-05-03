# Computer

A simple 8-bit emulated computer with its own instruction set and assembler.

# Getting Started

## Entering the development environment

```sh
nix develop
```

## Building the emulator

```sh
make build
```

## Compiling the assembly files into binary

```
python3 assembler.py <asm_source> <target_binary>
```

## Example
```sh
user$ python3 assembler.py decrease.asm decrease.bin
user$ ./cpu decrease.bin
11
10
9
8
7
6
5
4
3
2
1
0
HLT at 12
Execution finished!
```
