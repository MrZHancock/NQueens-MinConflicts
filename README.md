# n-Queens Solver (using min-conflicts)

If you are using a compiler other than GCC 12, you will
need to update the first line of the makefile.

If your compiler does not support the C++20 standard,
then you will also need to remove `-std=c++20`
from the second line of the makefile.
Your compiler must support at least the C++11 standard.

To compile the code, simply enter `make queens` in your
terminal (after navigating to the appropriate
directory).  After compiling, use `./Queens` to
generate a solution to the 8-queens puzzle.  For other
values of $n$, use `./Queens n` where `n` is a positive
integer.
