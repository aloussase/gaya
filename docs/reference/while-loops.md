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

## Break and continue

There are no `break` and `continue`. Gaya doesn't have any non-local
control-flow statements in order to favor structured programming. If you think
this is too much of a problem, feel free to open an issue on the
[repo](https://github.com/aloussase/gaya) about it.

## For loops

There are no for-loops in Gaya. If you need to iterate over a sequence, use
`seq.foreach`:

```
seq.foreach((1, 2, 3)) { n =>
  io.println(n)
}
```
