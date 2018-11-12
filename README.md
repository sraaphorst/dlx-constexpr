# dlx_constexpr

**Current status:** Complete, version 1.0.0.

This is a project that I wanted to work on for a long time, but I wasn't sure it would even be possible, i.e. a `constexpr` (read: compile-time evaluated) implementation of Donald Knuth's DLX / dancing links / Algorithm X:

https://en.wikipedia.org/wiki/Dancing_Links

This is a technique that is commonly and efficiently used to solve exact cover problems:

https://en.wikipedia.org/wiki/Exact_cover

Sudoku boards can be modeled as exact cover problems, and thus, DLX has become one of the main techniques to solve Sudoku boards, as it takes fractions of a megasecond, even for the most difficult of boards.

C++14/17 programming in general - and `constexpr` / template-metaprogramming in particular - are two things that I find incredibly fascinating: hence my realization of this `constexpr` implementation of DLX over the course of a weekend.

**NOTE** that you do not need to use DLX in a `constexpr` context. You can use the algorithm as a run-time implementaton of DLX as well.

## How to use it

The entire library is small and based on a single header, namely `dlx_constexpr.h`:

https://github.com/sraaphorst/dlx_constexpr/blob/master/dlx_contexpr.h

Simply drag this wherever you with is your source code tree, formulate your problem as an exact cover problem, and you're good to go. There are no dependencies or prerequisites apart from C++17. It may or may not work with your compiler: I have found that GCC is much more flexible with regards to `constexpr`, and clang far less so. This was tested with GCC 8.0, where it works perfectly; it produces errors in `constexpr` mode with clang 6.0.1.

## Examples

The rest of the code in this project comprises examples to show how easy it is to use `dlx_constexpr`, by modeling problems as Catch2 test cases, which you can see in the `test` folder:

https://github.com/sraaphorst/dlx_constexpr/tree/master/test

I looked at two problems in particular that lend themselves well to being solved by exact cover and DLX:

1. Solving sudoku. `dlx_constexpr` can solve the current most difficult Sudoku board known near-instantaneously at run-time, and unlike my previous project that used a brute-force backtracking approach to solve Sudoku boards specifically in a `constexpr` way, the code compiles quickly. (Brute force technique: more than 20 minutes. DLX technique: several seconds.)

2. `t-(v, k, 1)` combinatorial designs. These are a generalization of block designs and Steiner systems, which you can read about on Wikipedia at the link below. Currently, the test cases comprise producing several Steiner triple systems and Steiner quadruple systems.

https://en.wikipedia.org/wiki/Block_design#Generalization:_t-designs
