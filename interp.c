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
#define CONST(c) ((c) >> 3)
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
     *
     * NOTE: on stack showcases lowest TOPST means highest on stack
     */
    Rep,
    /*
     * EEEEX Ext
     * TOPST OOO
     *
     * If X is set then extend an op with val=EEEETOPST.
     *
     * Else modify top of the stack by setting TOP to EEE,
     * which is useful for pushing extended instructions
     *
     * Given X=1, and EEEE=0, this acts as an "execution" of top of the stack
     *
     * This allows to use dynamic stack values for operations that cannot
     */
    Ext,
    /*
     * EEEEE Cop
     * TOPST OOO
     *
     * If TOPST is nonzero, do op with val=EEEEE
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

void execute_op(uint8_t op, uint32_t val, uint8_t *mem, uint8_t *st,
                int *st_idx) {
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
        /* TODO: The pop may be an issue for chaining */
        *st_idx -= 1;
        uint8_t op = st[*st_idx + 1] & OP_MASK;
        uint32_t reps = CONST(st[*st_idx + 1]);
        for (uint32_t i = 0; i < reps; i++) {
            execute_op(op, val, mem, st, st_idx);
        }
        break;
    case Ext:
        if (val & 1) {
            *st_idx -= 1;
            execute_op(st[*st_idx + 1] & OP_MASK,
                       ((val >> 1) << 5) | CONST(st[*st_idx + 1]), mem, st,
                       st_idx);
        } else {
            st[*st_idx] |= (val >> 1) << 5;
        }
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
            typeof(*st) v = st[*st_idx];
            st[*st_idx] = mem[(val << (sizeof(*st) * 8)) | v];
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

int interpret(const uint8_t *in, uint32_t size) {
    const uint32_t st_limit = 100;
    uint8_t st[st_limit];
    int st_idx = 0;

    const uint32_t mem_limit = 1000;
    uint8_t mem[mem_limit];

    memset(st, 0, sizeof(*st) * st_limit);
    memset(mem, 0, sizeof(*mem) * mem_limit);

    for (uint32_t i = 0; i < size; i++) {
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

    uint8_t *buf = malloc(sbuf.st_size);

    fread(buf, 1, sbuf.st_size, out);

    interpret(buf, sbuf.st_size);

    free(buf);
    fclose(out);
    return 0;
}
