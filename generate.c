/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

/*
 * Bytecode is defined as a byte according to schema:
 * CCCCC OOO
 * Const Op
 * Where `Const` can represent a variety of values
 * `Op` on the other hand is one of the below
 */
#define OP_MASK 0b00000111
#define CONST(c) (c >> 3)
enum bytecode {
    Push = 0,
    Pop, /* Pop exactly how much you ask for */
    Add, /* Add top of stack with const */
    Neg,
    Cop, /* Uses a second conditional op with a small constant CC OOO Cop */
    In,  /* Choose input (currenlty term) */
    Out, /* Choose output (currenlty term) */
};

void execute_op(char op, char val, char *st, int *st_idx) {
    switch (op) {
    case Push:
        *st_idx += 1;
        st[*st_idx] = val;
        break;
    case Pop:
        *st_idx -= val;
        break;
    case Add:
        st[*st_idx] += val;
        break;
    case Neg:
        st[*st_idx] = -st[*st_idx];
        break;
    case Cop:
        if (st[*st_idx] > 0) {
            execute_op(val & OP_MASK, CONST(val), st, st_idx);
        }
        break;
    case In:
        *st_idx += 1;
        st[*st_idx] = getc(stdin);
        break;
    case Out:
        printf("%d\n", st[*st_idx]);
        break;
    }
}

int interpret(const char *in, int size) {
    const int st_limit = 100;
    char st[st_limit];
    memset(st, 0, st_limit);
    int st_idx = 0;
    for (int i = 0; i < size; ++i) {
        execute_op(*in & OP_MASK, CONST(*in), st, &st_idx);
        ++in;
        if (st_idx < 0) {
            puts("Overcame the stack's lower limit");
            return st_idx;
        } else if (st_idx >= st_limit) {
            puts("Overcame the stack's upper limit");
            return st_idx;
        }
    }
    return 0;
}

#define BUF_SIZE 100

int main(void) {
    FILE *out = fopen("out.byt", "w");

    char *buf = malloc(BUF_SIZE);

    int i = 0;

    buf[i++] = 0b11000 | Push;
    buf[i++] = 0b11000 | Add;
    buf[i++] = Out;
    buf[i++] = 0b1000 | Pop;
    buf[i++] = ((0b1000 | Add) << 3) | Cop;
    buf[i++] = Out;

    interpret(buf, i);

    fwrite(buf, 1, BUF_SIZE - 1, out);

    free(buf);
    fclose(out);
    return 0;
}
