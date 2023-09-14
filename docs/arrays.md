# Arrays

Gaya arrays are backed by C++ `std::vector`.

Arrays are defined as a list of comma separated expressions between
parenthesis:

```ocaml
(1, 2, 3)

(io.println("Hello"), 1 + 2)
```

Arrays are callable objects. The call operation corresponds to
indexing in other languages:

```ocaml
(1, 2, 3)(0)
```

Providing an invalid index will result in a runtime error.
