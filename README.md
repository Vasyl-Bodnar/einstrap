# einstrap

A small 8 bit instruction bytecode. Capable of supporting higher-bit system however.

Note that it is bound to change heavily to fit my usecases.

## Description

Bytecode is defined as a byte according to schema:

```
VVVVV OOO
val   op
```

Where `val` represents a variety of values. 
`op` on the other hand is one of the below with their semantics:

- Push: put `val` on top of the stack
- Pop: remove `val` number of stack entries
- Add: add `val` to top of the stack
- Rep: repeat an op a number of times
- Ext: extend an op to more bits
- Cop: conditional op
- Load: push from memory or using getc for val=0
- Store: output the number on top of the stack to memory or terminal for val=0

Most operations use extended code with the stack in some way, which is according to the below:

```
EEEEE OOO
TOPSTACK
```

e.g. Rep:

```
EEEEE Rep
TOPST OOO
```

More details are provided in the code, which is always more up to date.

## Compile and Run

The program provided is a C interpreter implementation and a Guile Scheme script.

You can use the script to generate some simple bytecode, e.g. `guile generate.scm`.

For C, this utilizes [xmake](https://xmake.io/), which might not be present on your system.

You can use `xmake` or `xmake run` to build and/or run the project. 

You can also configure it for cross-compilation using e.g. `xmake f -p windows --toolchain=mingw`, assuming you possess the required toolchains and targets.

In case you do not wish to use `xmake`, you can just do a simple `cc interp.c -O2 -Wall` and run the `a.out`.
