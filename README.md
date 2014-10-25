# Marvin AI
## A C extension AI for Python-implemented 2048

C++ code from [nneoneo](https://github.com/nneonneo "nneoneo")'s [2048 AI](https://github.com/nneonneo/2048-ai "2048-ai") ported to vanilla C and wrapped in a Python module.

**N.B.** This is not my AI! The original AI written by [nneoneo](https://github.com/nneonneo "nneoneo"). This is merely my C / Python FFI port of his algorithm. For nefarious purposes.


### Quickstart

The source in `MARVIN/` should be ready to build out of the box:

```
% pushd MARVIN
% make
% popd
```

This will build the Python module from source.

### Testing the Install

Only the lamest of tests are provided. You should be able to run `check.py` and see that the answer is, in fact, `1`.

```
% python check.py
0,2,4,1
1,0,2,4
4,0,1,2
5,4,3,2
MOVE: 1
```

Then you're good to go! Hopefully...


### Playing with Marvin AI

`MarvinAI.py` is provided as a functional AI class that wraps the `MarvinAI` library. This can be modified in pure Python if so desired.


### Modifications

Major modifications made to the AI for misc. reasons: (i) No C standard implementation of the C++ `map` templates/classes, so caching is disabled; (ii) Max depth changed to speed up game response time.

See ll. 376-378 for additional details.

```
/* state->depth_limit =
   max(3, count_distinct_tiles(board) - 2); */
state->depth_limit = 3; 
```

Currently the max depth is set to 3, where as previously the AI would searc to depths 5-8. This is a response-time optimization.


### Build Notes

The `Makefile` is specifically intended to target Apple's native installed Python distro. To use this software, you might need to target the correct `Python.h` header in your O/S install.
