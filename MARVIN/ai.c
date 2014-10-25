#ifndef __AI_H__
#include "ai.h"
#endif

static const float CPROB_THRESH_BASE = 0.0001f;

static inline
float
min(float a, float b)
{
  if (a <= b) return a;
  else        return b;
}

static inline
float
max(float a, float b)
{
  if (a > b) return a;
  else       return b;
}

static inline
board_t
unpack_col(row_t row)
{
  board_t tmp = row;
  return
    (tmp | (tmp << 12ULL) | (tmp << 24ULL) | (tmp << 36ULL)) & COL_MASK;
}

static inline
row_t
reverse_row(row_t row)
{
  return
    (row >> 12) | ((row >> 4) & 0x00F0) | ((row << 4) & 0x0F00) | (row << 12);
}

static inline
board_t
transpose(board_t x)
{
  board_t a1 = x & 0xF0F00F0FF0F00F0FULL;
  board_t a2 = x & 0x0000F0F00000F0F0ULL;
  board_t a3 = x & 0x0F0F00000F0F0000ULL;
  board_t a = a1 | (a2 << 12) | (a3 >> 12);
  board_t b1 = a & 0xFF00FF0000FF00FFULL;
  board_t b2 = a & 0x00FF00FF00000000ULL;
  board_t b3 = a & 0x00000000FF00FF00ULL;
  return b1 | (b2 >> 24) | (b3 << 24);
}

static
int
count_empty(board_t x)
{
  x |= (x >> 2) & 0x3333333333333333ULL;
  x |= (x >> 1);
  x = ~x & 0x1111111111111111ULL;

  // At this point each nibble is:
  //  0 if the original nibble was non-zero
  //  1 if the original nibble was zero
  // Next sum them all

  x += x >> 32;
  x += x >> 16;
  x += x >>  8;
  x += x >>  4; // this can overflow to the next nibble if 16 empty positions
  return x & 0xf;
}

static row_t   row_left_table   [65536];
static row_t   row_right_table  [65536];
static board_t col_up_table     [65536];
static board_t col_down_table   [65536];
static float   heur_score_table [65536];

static const float SCORE_LOST_PENALTY        = 200000.0f;
static const float SCORE_MONOTONICITY_POWER  = 4.0f;
static const float SCORE_MONOTONICITY_WEIGHT = 47.0f;
static const float SCORE_SUM_POWER           = 3.5f;
static const float SCORE_SUM_WEIGHT          = 11.0f;
static const float SCORE_MERGES_WEIGHT       = 700.0f;
static const float SCORE_EMPTY_WEIGHT        = 270.0f;

static
void
_add_row_score(unsigned row, unsigned * line)
{
  int empty = 0;
  int merges = 0;
  float sum = 0;

  int prev = 0;
  int counter = 0;
  int i,rank;

  float monotonicity_left  = 0;
  float monotonicity_right = 0;

  for (i=0; i<4; ++i) {
    rank = line[i];
    sum += pow(rank, SCORE_SUM_POWER);

    if (rank == 0) {
      empty++;
    } else {
      if (prev == rank) {
        counter++;
      } else if (counter > 0) {
        merges += 1 + counter;
        counter = 0;
      }
      prev = rank;
    }
  }

  if (counter > 0) {
    merges += 1 + counter;
  }
  
  for (i=1; i<4; ++i) {
    if (line[i-1] > line[i]) {
      monotonicity_left +=
        pow(line[i-1], SCORE_MONOTONICITY_POWER) - \
        pow(line[i], SCORE_MONOTONICITY_POWER);
    } else {
      monotonicity_right +=
        pow(line[i], SCORE_MONOTONICITY_POWER) - \
        pow(line[i-1], SCORE_MONOTONICITY_POWER);
    }
  }
  
  heur_score_table[row] =
    SCORE_LOST_PENALTY +
    SCORE_EMPTY_WEIGHT * empty +
    SCORE_MERGES_WEIGHT * merges -
    SCORE_MONOTONICITY_WEIGHT * min(monotonicity_left, monotonicity_right) -
    SCORE_SUM_WEIGHT * sum;
}

static
void
_add_row_trans(unsigned row, unsigned *line)
{
  int i,j;
  for (i=0; i<3; ++i) {
    for (j=i+1; j<4; ++j) {
      if (line[j] != 0) break;
    }
    if (j == 4) break;    
    if (line[i] == 0) {
      line[i] = line[j];
      line[j] = 0;
      i--;
    } else if (line[i] == line[j] && line[i] != 0xf) {
      line[i]++;
      line[j] = 0;
    }
  }

  row_t result =
    (line[0] <<  0) | (line[1] <<  4) | (line[2] <<  8) | (line[3] << 12);
  row_t rev_result = reverse_row(result);
  unsigned rev_row = reverse_row(row);

  row_left_table [    row] =                row  ^                result;
  row_right_table[rev_row] =            rev_row  ^            rev_result;
  col_up_table   [    row] = unpack_col(    row) ^ unpack_col(    result);
  col_down_table [rev_row] = unpack_col(rev_row) ^ unpack_col(rev_result);
}

void
init_tables(void)
{
  unsigned row;
  unsigned line[4];

  for (row = 0; row < 65536; ++row) {
    line[0] = (row >>  0) & 0xf;
    line[1] = (row >>  4) & 0xf;
    line[2] = (row >>  8) & 0xf;
    line[3] = (row >> 12) & 0xf;

    _add_row_score(row, (unsigned *) line);
    _add_row_trans(row, (unsigned *) line);
  }
}

static
inline
board_t
execute_move_0(board_t board)
{
  board_t ret = board;
  board_t t = transpose(board);
  ret ^= col_up_table[(t >>  0) & ROW_MASK] <<  0;
  ret ^= col_up_table[(t >> 16) & ROW_MASK] <<  4;
  ret ^= col_up_table[(t >> 32) & ROW_MASK] <<  8;
  ret ^= col_up_table[(t >> 48) & ROW_MASK] << 12;
  return ret;
}

static inline
board_t
execute_move_1(board_t board)
{
  board_t ret = board;
  board_t t = transpose(board);
  ret ^= col_down_table[(t >>  0) & ROW_MASK] <<  0;
  ret ^= col_down_table[(t >> 16) & ROW_MASK] <<  4;
  ret ^= col_down_table[(t >> 32) & ROW_MASK] <<  8;
  ret ^= col_down_table[(t >> 48) & ROW_MASK] << 12;
  return ret;
}

static inline
board_t
execute_move_2(board_t board)
{
  board_t ret = board;
  ret ^= (board_t) (row_left_table[(board >>  0) & ROW_MASK]) <<  0;
  ret ^= (board_t) (row_left_table[(board >> 16) & ROW_MASK]) << 16;
  ret ^= (board_t) (row_left_table[(board >> 32) & ROW_MASK]) << 32;
  ret ^= (board_t) (row_left_table[(board >> 48) & ROW_MASK]) << 48;
  return ret;
}

static inline
board_t
execute_move_3(board_t board)
{
  board_t ret = board;
  ret ^= (board_t) (row_right_table[(board >>  0) & ROW_MASK]) <<  0;
  ret ^= (board_t) (row_right_table[(board >> 16) & ROW_MASK]) << 16;
  ret ^= (board_t) (row_right_table[(board >> 32) & ROW_MASK]) << 32;
  ret ^= (board_t) (row_right_table[(board >> 48) & ROW_MASK]) << 48;
  return ret;
}

static inline
board_t
execute_move(int move, board_t board)
{
  switch(move) {
  case 0: // up
    return execute_move_0(board);
  case 1: // down
    return execute_move_1(board);
  case 2: // left
    return execute_move_2(board);
  case 3: // right
    return execute_move_3(board);
  default:
    return ~0ULL;
  }
}

static inline
int
count_distinct_tiles(board_t board)
{
  uint16_t bitset = 0;
  while (board) {
    bitset |= 1<<(board & 0xf);
    board >>= 4;
  }

  bitset >>= 1;

  int count = 0;
  while (bitset) {
    bitset &= bitset - 1;
    count++;
  }
  return count;
}

typedef struct _STATE {
  int depth_limit;
  int maxdepth;
  int curdepth;

  unsigned long moves_evaled;
} state_t;

static
float
score_helper(board_t board, const float* table)
{
  return
    table[(board >>  0) & ROW_MASK] +
    table[(board >> 16) & ROW_MASK] +
    table[(board >> 32) & ROW_MASK] +
    table[(board >> 48) & ROW_MASK];
}

static
float
score_heur_board(board_t board)
{
  return
    score_helper(          board , heur_score_table) +
    score_helper(transpose(board), heur_score_table);
}

static float score_tilechoose_node(state_t *, board_t, float);
static float score_move_node(state_t *, board_t, float);

static
float
score_move_node(state_t *state, board_t board, float cprob)
{
  int move;
  float best = 0.0f;

  state->curdepth++;
  for (move=0; move<4; ++move) {
    board_t newboard = execute_move(move, board);
    state->moves_evaled++;

    if (board != newboard) {
      best = max(best, score_tilechoose_node(state, newboard, cprob));
    }
  }
  state->curdepth--;

  return best;
}

static
float
score_tilechoose_node(state_t *state, board_t board, float cprob)
{
  if (cprob < CPROB_THRESH_BASE || state->curdepth >= state->depth_limit) {
    state->maxdepth = max(state->curdepth, state->maxdepth);
    return score_heur_board(board);
  }

  int num_open = count_empty(board);
  cprob /= num_open;

  float res = 0.0f;
  board_t tmp = board;
  board_t tile_2 = 1;
  while (tile_2) {
    if ((tmp & 0xf) == 0) {
      res += score_move_node(state, board |  tile_2      , cprob * 0.9f) * 0.9f;
      res += score_move_node(state, board | (tile_2 << 1), cprob * 0.1f) * 0.1f;
    }
    tmp >>= 4;
    tile_2 <<= 4;
  }
  res = res / num_open;

  return res;
}

static
float
_score_toplevel_move(state_t *state, board_t board, int move)
{
  board_t newboard = execute_move(move, board);
  if (board == newboard) return 0;
  return score_tilechoose_node(state, newboard, 1.0f) + 1e-6;
}

static
float
score_toplevel_move(state_t *state, board_t board, int move)
{
  float res;

  /* state->depth_limit =
     max(3, count_distinct_tiles(board) - 2); */
  state->depth_limit = 3;
  res = _score_toplevel_move(state, board, move);

  return res;
}

int
find_best_move(board_t board)
{
  int move;
  float best = 0;
  int bestmove = -1;

  state_t *state = (state_t *) malloc(sizeof(state_t));
  memset((void *) state, 0x00, sizeof(state_t));

  for(move=0; move<4; ++move) {
    float res = score_toplevel_move(state, board, move);

    if (res > best) {
      best = res;
      bestmove = move;
    }
  }

  free(state);
  return bestmove;
}
