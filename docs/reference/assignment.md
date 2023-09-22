# Assignment

The assignment operator `<-` can be used to mutate local variables (those
introduced with `let`). It cannot be used on global definitions or function
parameters.

```ocaml
let x = 0 in do
  x <- 10
  io.println(x)
end.
```

The assignment operator can only be used in _local statements_, meaning `do`
blocks, the body of `while` loops, and the argument to `perform`.
