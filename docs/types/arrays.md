# Arrays

Gaya arrays are backed by C++ `std::vector`.

Arrays are defined as a list of comma separated expressions between
parenthesis:

```ocaml
(1, 2, 3).

(io.println("Hello"), 1 + 2).
```

Arrays are callable objects. The call operation corresponds to indexing in
other languages:

```ocaml
(1, 2, 3)(0)
```

Providing an invalid index will result in a runtime error.

Arrays participate in the sequence protocol. That means you can use
`tosequence` and `seq.next` on them. Also, all functions in the sequence
library (`runtime/sequences.gaya`) work on arrays.

For operations available on arrays, see `runtime/arrays.gaya` and
`include/builtins/array.hpp`.
