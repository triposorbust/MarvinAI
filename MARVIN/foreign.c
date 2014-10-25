#ifndef __FOREIGN_H__
#include "foreign.h"
#endif

#undef __DEBUG_FLAG__
#ifdef __DEBUG_FLAG__
static
void
print_board(board_t board)
{
  int i,j;
  for(i=0; i<4; i++) {
    for(j=0; j<4; j++) {
      printf("%c", "0123456789abcdef"[(board)&0xf]);
      board >>= 4;
    }
    printf("\n");
  }
}
#endif

static
board_t
build_board(PyObject *map)
{
  board_t board;
  PyObject *row,*pval;
  Py_ssize_t i,j,n;
  long cval;
  int k;

  memset((void *) &board, 0x00, sizeof(board_t));
  n = PyList_Size(map);
  for (i=n-1; i>=0; --i) {
    row = PyList_GetItem(map, i);
    for (j=n-1; j>=0; --j) {
      pval = PyList_GetItem(row, j);
      cval = PyInt_AsLong(pval);

      k = 0;
      if (0 != cval)
        while (0x1 != (cval >> ++k))
          ; /* so cryptic! */

      board |= (k & 0xf);
      if (j != 0 || i != 0)
        board = board << 4;
    }
  }
  return board;
}

PyObject *
MarvinAI_initialize(PyObject *self,
                    PyObject *args)
{
  (void) args;
  (void) self;

  init_tables();

  Py_INCREF(Py_None);
  return Py_None;
}

PyObject *
MarvinAI_evaluate(PyObject *self,
                  PyObject *args)
{
  int move;
  PyObject * map;
  if (! PyArg_Parse( args, "O!", &PyList_Type, &map )) {
    return NULL;
  }
  (void) self;

  board_t board = build_board(map);
  move = find_best_move(board);

  return Py_BuildValue("i", move);
}
