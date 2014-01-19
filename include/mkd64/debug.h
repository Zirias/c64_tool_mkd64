#ifndef MKD64_DEBUG_H
#define MKD64_DEBUG_H

/** Macros for debugging output.
 * These macros should be used to output unimportant messages, that might be
 * useful during debugging.
 * @file
 */

#ifdef DEBUG
#include <stdio.h>
#define DBG(x) fprintf(stderr, "[DEBUG] %s\n", x)
#define DBGd1(x,a) fprintf(stderr, "[DEBUG] %s %d\n", x, a)
#define DBGd2(x,a,b) fprintf(stderr, "[DEBUG] %s %d %d\n", x, a, b)
#define DBGs1(x,a) fprintf(stderr, "[DEBUG] %s %s\n", x, a)
#define DBGn(x) fprintf(stderr, "[DEBUG] %s", x)
#define DBGnd1(x,a) fprintf(stderr, "[DEBUG] %s %d", x, a)
#define DBGnd2(x,a,b) fprintf(stderr, "[DEBUG] %s %d %d", x, a, b)
#define DBGns1(x,a) fprintf(stderr, "[DEBUG] %s %s", x, a)
#else

/** Print debugging message.
 * @param x the message
 */
#define DBG(x)

/** Print debugging message and a number.
 * @param x the message
 * @param a the number
 */
#define DBGd1(x,a)

/** Print debugging message and two numbers.
 * @param x the message
 * @param a first number
 * @param b second number
 */
#define DBGd2(x,a,b)

/** Print debugging message and an extra string.
 * @param x the message
 * @param a the extra string
 */
#define DBGs1(x,a)

/** Print debugging message without newline.
 * @param x the message
 */
#define DBGn(x)

/** Print debugging message and a number without newline.
 * @param x the message
 * @param a the number
 */
#define DBGnd1(x,a)

/** Print debugging message and two numbers without newline.
 * @param x the message
 * @param a first number
 * @param b second number
 */
#define DBGnd2(x,a,b)

/** Print debugging message and an extra string without newline.
 * @param x the message
 * @param a the extra string
 */
#define DBGns1(x,a)
#endif

#endif
/* vim: et:si:ts=8:sts=4:sw=4
*/
