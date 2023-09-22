# Closures

Closure are functions that capture their environment. For a thorough
introduction to closures, check out
[this](<https://en.wikipedia.org/wiki/Closure_(computer_programming)>) wikipedia
article.

An example from that article translated to Gaya:

```ocaml
f :: { x =>
  { y => x + y }
}

a :: f(1)

io.println(a(5)). (* Output: 6 *)
```

In this example `f` returns a closure that captures the parameter `x`.

The combination of closures and assignment allows us to simulate stateful
objects:

```ocaml
mkCounter :: { =>
  let count = 0 in { =>
    let previous = count in do
      count <- count + 1
      previous
    done
  }
}

counter :: mkCounter()

io.println(counter()). (* Output: 0 *)
io.println(counter()). (* Output: 1 *)
```

Really cool ;). See the `object.gaya` example in the `examples` directory of
the repository's root to see how we can simulate message passing using just
closures.
