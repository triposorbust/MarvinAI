#ifndef __AI_H__
#define __AI_H__

#ifndef __STDLIB_H__
#define __STDLIB_H__
#include <stdlib.h>
#endif

#ifndef __STDINT_H__
#define __STDINT_H__
#include <stdint.h>
#endif

#ifndef __STDIO_H__
#define __STDIO_H__
#include <stdio.h>
#endif

#ifndef __XMATH_H__
#define __XMATH_H__
#include <math.h>
#endif

#ifndef __STRING_H__
#define __STRING_H__
#include <string.h>
#endif

#ifndef __TIME_H__
#define __TIME_H__
#include <time.h>
#endif

typedef uint64_t board_t;
typedef uint16_t row_t;

static const board_t ROW_MASK = 0xFFFFULL;
static const board_t COL_MASK = 0x000F000F000F000FULL;

int find_best_move(board_t);
void init_tables(void);

#endif
