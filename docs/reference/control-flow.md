# Control Flow

## While loops

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

Finally, you can also use an initialization statement, which would make Gaya's
while loops the essentially the same as C's for-loops:

```
while let i = 0 : i < 10 : i <- i + 1
  io.println(i).
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

For loops in Gaya are used to iterate over sequences. Just like with while
loops, the body of a for-loop should consist only of statements. For example:

```
for x in 42
  io.println(x).
end
```

This would print the numbers from 0 to 41.

Note that the above example may also be written by using the `seq.foreach`
function from the `sequences` library:

```
seq.foreach(42) { x =>
  io.println(x).
}.
```

The only difference is that the for-loop is a statement and the function call
is an expression, so they are used differently in different contexts.
