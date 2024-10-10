/*
 * FILE      code\gfs\include\gfs\random.h
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 */
 #include "gfs/random.h"

//
// Reference:
//     - https://en.wikipedia.org/wiki/Xorshift
//     - https://www.cs.yale.edu/homes/aspnes/pinewiki/C(2f)Randomization.html
//     - https://youtu.be/hHrndSl2xSU?si=UH1odFnxnML4mkRz

u32
XOrShift32GetNext(XOrShift32State *state) {
	/* Algorithm "xor" from p. 4 of Marsaglia, "Xorshift RNGs" */

	u32 x = state->value;
	x ^= x << 13;
	x ^= x >> 17;
	x ^= x << 5;
	state->value = x;
	return x;
}

i32
RandI32(i32 min, i32 max) {
    static XOrShift32State state = { .value = 1234 } ;  // XXX
    return min + XOrShift32GetNext(&state) % (max - min + 1);
}

bool
RandBool() {
    return RandI32(0, 1) == 0;
}
