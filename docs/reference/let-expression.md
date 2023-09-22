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
