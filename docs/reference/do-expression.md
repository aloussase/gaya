# Do Expression

A `do` expression is simply a way to group statements. All but the last element
in the body _must_ be statements. If the last element is an expression, its
value is returned, else, if it's a statement, unit is returned.

```ocaml
(* Results in 42*)
do
  io.println("A statement").
  42
end

(* Results in unit *)
do
  let x = 0 in perform
  while x < 10
    &x <- x + 1
  end.
end
```
