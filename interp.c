/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

/*
 * Bytecode is typically defined as a byte according to schema:
 * VVVVV OOO
 * val   op
 *
 * Where `val` can represent a variety of values
 * `op` on the other hand is one of the below
 *
 * Most operations can be extended, which will also utilize the stack
 *
 * Though the bytecode is limited to 8 bits per instruction,
 * realistically there is no limit due to chaining of repetitions and extensions
 *
 * Note that memory and stack do not have to be bytes, and the system supports
 * an infinite amount of bits per space
 *
 * Specific details are provided per instruction
 */
#define OP_MASK 0b00000111
#define CONST(c) (c >> 3)
enum bytecode {
    /*
     * VVVVV Push
     * Push the val onto the stack
     * Can be extended to push higher values
     * Can be repeated to dup the val
     */
    Push = 0,
    /*
     * VVVVV Pop
     * Pop the val values off the stack
     * Can be extended to pop more values
     * Can be repeated to pop multiple of val off the stack
     *
     * NOTE: A little unnecessery, can just merge it into push
     */
    Pop,
    /*
     * VVVVV Add
     * Add the val to the top of the stack
     * Can be extended to add higher values
     */
    Add,
    /*
     * EEEEE Rep
     * TOPST OOO
     *
     * Repeat an op TOPST times with val=EEEEE
     * Can repeat itself,
     * e.g. Rep Rep will have TOPST * TOPST repetitions of op with val=EEEEE
     *
     * This allows to use dynamic stack values for operations that cannot
     *
     * NOTE: using TOPSTACK implies a pop
     */
    Rep,
    /*
     * EEEEE Ext
     * TOPST OOO
     *
     * Extend an op with val=EEEEETOPST
     * Can extend itself, e.g. Ext Ext will have val=EEEEETOPSTTOPST for the op
     *
     * This allows to use dynamic stack values for operations that cannot
     *
     * NOTE: The lowest TOPST is the one down the chain,
     * e.g. EEEEE Ext
     *      TOPS1 Ext
     *      TOPS2 OOO
     * will have val=EEEEETOPS1TOPS2 for op
     */
    Ext,
    /*
     * EEEEE Cop
     * TOPST OOO
     *
     * If TOPST is zero, do op with val=EEEEE
     *
     * Can be extended to increase maximum value
     * Can be repeated for side effects
     */
    Cop,
    /*
     * EEEEE Load
     * TOPSTACK
     *
     * Load value from val=EEEEETOPSTACK addr onto stack
     * Can be extended to use a larger address
     * Can be repeated to dup
     */
    Load,
    /*
     * EEEEE Store
     * TOPSTACK
     *
     * Store top of the stack at val=EEEEE addr
     * Can be extended to use a larger address
     */
    Store,
};

void execute_op(char op, int val, int *mem, int *st, int *st_idx) {
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
    case Rep:
        *st_idx -= 1;
        for (typeof(val) i = 0, reps = CONST(st[*st_idx + 1]); i < reps; i++) {
            execute_op(st[*st_idx] & OP_MASK, val, mem, st, st_idx);
        }
        break;
    case Ext:
        *st_idx -= 1;
        execute_op(st[*st_idx + 1] & OP_MASK,
                   (val << 8) | CONST(st[*st_idx + 1]), mem, st, st_idx);
        break;
    case Cop:
        *st_idx -= 1;
        if (CONST(st[*st_idx + 1]) > 0) {
            execute_op(st[*st_idx + 1] & OP_MASK, val, mem, st, st_idx);
        }
        break;
    case Load:
        // Currently val of 0 is special for calling getc (testing purposes)
        if (val) {
            typeof(val) v = st[*st_idx];
            st[*st_idx] = mem[(val << 8) | v];
        } else {
            st[*st_idx] = getc(stdin);
        }
        break;
    case Store:
        // Currently val of 0 is special for calling printf (testing purposes)
        if (val) {
            mem[val] = st[*st_idx];
        } else {
            printf("%d\n", st[*st_idx]);
        }
        *st_idx -= 1;
        break;
    }
}

int interpret(const char *in, int size) {
    const int st_limit = 100;
    int st[st_limit];
    int st_idx = 0;

    const int mem_limit = 1000;
    int mem[mem_limit];

    memset(st, 0, sizeof(*st) * st_limit);
    memset(mem, 0, sizeof(*mem) * mem_limit);

    for (int i = 0; i < size; i++) {
        execute_op(*in & OP_MASK, CONST(*in), mem, st, &st_idx);
        in++;
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

int main(void) {
    FILE *out = fopen("out.byt", "r");

    struct stat sbuf;

    fstat(fileno(out), &sbuf);

    char *buf = malloc(sbuf.st_size);

    fread(buf, 1, sbuf.st_size, out);

    interpret(buf, sbuf.st_size);

    free(buf);
    fclose(out);
    return 0;
}
