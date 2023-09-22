# While loops

While loops work as you'd expect in other programming languages:

```ocaml
let x = 0 in perform
while x < 10
  io.println(x).
  x <- x + 1
end
```

An optional continuation statement may be put after the while condition like in
Zig. So the last example could be rewritten to:

```ocaml
let x = 0 in perform
while x < 10 : x <- x + 1
  io.println(x).
end
```

The body of the while loop should consist entirely of statements. This means
that if you call a function, you need to discard its result explicitely if you
are not going to use it.
