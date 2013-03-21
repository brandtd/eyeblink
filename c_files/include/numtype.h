#ifndef NUMTYPE_H
#define NUMTYPE_H

/**
 * All code dealing with floating point numbers must include this file at some
 * point. This file contains the definition of NUMTYPE, the datatype to use for
 * floating point numbers in our algorithms.
 *
 * NUMTYPE will always be either 'double' or 'float', depending on whether or
 * not the macro USE_SINGLE is defined. If USE_SINGLE is not defined, the
 * number type defaults to 'double'. If USE_SINGLE is defined, then the number
 * type becomes 'float'.
 *
 * NOTE: the CUDA programs will not compile using double precision floating
 * point numbers as the number type. The trigonometric functions cannot handle
 * doubles.
 */
#ifdef USE_SINGLE
  #define NUMTYPE float
#else
  #define NUMTYPE double
#endif

/**
 * Define a macro based on the USE_SINGLE option. This way, if there is code
 * that depends on the USE_SINGLE switch, it can be contained in normal if,else
 * blocks. This is a best practice that ensures that all code is seen by the
 * compiler so there will be no surprises when a flag is/isn't defined. Any
 * modern compiler will compile out any unreachable code.
 */
#ifdef USE_SINGLE
  #define ISDEF_USE_SINGLE 1
#else
  #define ISDEF_USE_SINGLE 0
#endif

#endif
