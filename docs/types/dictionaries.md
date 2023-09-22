# Dictionary

Dictionaries are backed by
[`robin_hood::unordered_map`](https://github.com/martinus/robin-hood-hashing),
a fast and efficient C++ hash table implementation.

They are defined as a sequence of expression pairs separated by a `->` between
parenthesis.

```
("name" -> "Gaya", "species" -> "Cat").
```

The empty dictionary is denoted `(->)`, because `()` already stands for the
empty array.

Dictionaries are callable objects. Calling a dictionary will attempt to
retrieve the value for the corresponding key. If the attempt fails, unit if
returned.

```
("name" -> "Gaya")("name").     (* "Gaya" *)
("name" -> "Gaya")("species").  (* unit *)
```

Dictionaries participate in the sequence protocol as a stream of two element
arrays, each containing a corresponding key-value pair from the dictionary.

You can use `tosequence` and `seq.next` on them. Also, all functions in the
sequence library (`runtime/sequences.gaya`) work on dictionaries.

For operations available on dictionaries, see `runtime/dictionaries.gaya` and
`include/builtins/dict.hpp`.
