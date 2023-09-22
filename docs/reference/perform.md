# Discarding values and performing statements

To discard the result of an expression, which is mandatory if you are not using
it, you put a '.' at the end, just like semicolons in Rust for example.

On the other hand, `perform` allows you to _perform_ a single statement in an
expression context, returning `unit`.

This combination is useful in control structures that expect statements or
expressions, but you need to do the other. For example, `let` expects what
comes after `in` to be an expression, but it might be useful to put a statement
instead. This can be done in two ways.

First, with a `do` block:

```julia
let x = 0 in do
  while x < 10
    x <- x + 1
  end
end
```

Second, with `perform`:

```julia
let x = 0 in perform
while x < 10
  x <- x + 1
end
```

Both are semantically equivalent. Its a matter of taste which one you choose to
use.
