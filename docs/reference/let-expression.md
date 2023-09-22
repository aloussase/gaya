# Let Expression

`let` expressions are used to introduce local bindings. Each binding in the
bindings list is available to the next one, so you can do things like:

```
let x = 5, y = x + 1 in io.println(y). (* Output: 6 *)
```

A more OCaml style is also possible:

```
let x = 5 in
let y = x + 1 in
io.println(y).
```

What comes after the `let` keyword can be an arbitrary pattern, not just an
identifier. For example:

```
let xs = (1, 2, 3) in
let (x, y, z) = xs in do
  assert(x == 1).
  assert(y == 2).
  assert(z == 3).
end.

let "A" = "A" in unit.
let _ = "A" in unit.
let x = "A" in assert(x == "A").
let x = 12, 12 = x in assert(x == 12).
```

See the section on match expressions for a more detailed introduction to
pattern matching in Gaya.
