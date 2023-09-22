# Basic Syntax

## Coments

Comments are OCaml style:

```ocaml
(* This is a comment *)

(*
  Works across multiple lines as well.
*)
```

Not much more to comment here.

## Literals

### Strings

Gaya has string literals that support escape sequences (but no unicode):

```ocaml
"Hello world"

"Hello, world!\n"

"\x1b[31mHello Red\n\x1b[m"
```

### Numbers

Numbers map to C++ `double` values and work pretty much as you would expect in
other programming languages:

```ocaml
42
42.69
-42
```

There is no exponent notation though.

### Unit

The unit value is represented by the `unit` keyword.

```
unit
```

### Arrays

Arrays are comma-separated lists of expression enclosed in parenthesis:

```
(* The empty array. *)
()

(* Nesting is supported. *)
(1, 2, "Hello", (5, 6))
```

More on arrays on their dedicated chapter.

### Dictionaries

Dictionaries are much like arrays, but the values are key-value pairs separated
by a '->':

```
(* The empty dict. *)
(->)

(* Nesting is supported as well. *)
(1 -> 2, "Hello" -> "World", (3 -> 4))
```

## Keywords

Here's a list of Gaya keywords:

- and
- cases
- do
- end
- for
- given
- in
- include
- let
- not
- or
- otherwise
- perform
- unit
- when
- while

That's a grand total of 16 (13 if you don't count `and` and `or`, since they
are technically operators).
