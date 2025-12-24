# einstrap

A small 8 bit instruction bytecode. 

Note that it is bound to change heavily to fit my usecases.

## Description

Bytecode is defined as a byte according to schema:

```
CCCCC OOO
Const Op
```

Where `Const` represents a variety of values. 
`Op` on the other hand is one of the below with their semantics:

- Push, put `Const` on top
- Pop, remove `Const` number of stack entries (`Const=0` is NOP)
- Add, add `Const` to top of the stack
- Neg, negate top of the stack
- Cop, conditional op, `Const` is then `CC OOO`
- In, push using getc
- Out, output the number on top of the stack

## Compile and Run

This utilizes [xmake](https://xmake.io/), which might not be present on your system.

You can use `xmake` or `xmake run` to build and/or run the project. 

You can also configure it for cross-compilation using e.g. `xmake f -p windows --toolchain=mingw`, 
assuming you possess the required toolchains and targets.

In case you do not wish to use `xmake`, you can just do a simple `cc generate.c -O3 -Wall` and run the `a.out`.
