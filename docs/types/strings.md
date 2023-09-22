# String

Gaya strings are backed by C++ `std::string`.

Strings are defined as a sequence of characters between double quotes:

```ocaml
"Hello"

"Hello\nGaya!"

"Hello \x1b[31mGaya\x1b[m"
```

They support all escape sequences specified in the JSON grammar, except unicode
(which is to come some day, maybe).

Strings are callable objects. Calling a string is equivalent to the indexing
operation in other languages:

```ocaml
"Gaya"(0) (* G *)
```

Providing an invalid index will result in a runtime error.

Strings participate in the sequence protocol. That means you can use
`tosequence` and `seq.next` on them. Also, all functions in the sequence
library (`runtime/sequences.gaya`) work on strings.

For operations available on strings, see `runtime/strings.gaya` and
`include/builtins/string.hpp`.
