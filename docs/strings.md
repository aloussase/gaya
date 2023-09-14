# String

Gaya strings are backed by C++ `std::string`.

Strings are defined as a sequence of characters between double
quotes:

```ocaml
"Hello"

"Hello\nGaya!"

"Hello \x1b[31mGaya\x1b[m"
```

They support all escape sequences specified in the JSON grammar,
except unicode (which is to come some day).

Strings are callable objects. Calling a string is equivalent to the
indexing operation in other languages:

```ocaml
"Gaya"(0) (* G *)
```

Providing an invalid index will result in a runtime error.
