<p align="center">
  <img src="./assets/logo/logo256x256.png" alt="gaya-logo" />
</p>

<h3 align="center">Gaya</h3>

```ocaml
"  Gaya is cool  "
    |> string.trim(_)
    |> _ <> "!"
    |> string.split(_, " ")
```

Gaya is a scripting language I made for solving
[AoC](https://adventofcode.com/) problems in 2023. The name comes from my
favourite cat.

I wanted to make something that makes me feel pleasure every time I use it. I
hope this way I can give pleasure to you too.

#### Motivation

It all started when I was trying out [Typst](https://typst.app/). I was writing
a document for a hackathon I was organizing and I wanted to try its scripting
capabilities to solve the proposed exercises in the document itself.

I could solve all graph processing problems and at the end I thought: "This is
a very simple scripting language, and yet it was enough for solving these
problems, surely I can write such a language myself". And here we are now.

#### Features

- **_Blazingly Fast_ (for a tree-walk interpreter):** Gaya implements a number
  of optimizations that make it go brrr. Some of these optimizations are:

  - NaN Boxing
  - Uses an efficient hash table for the environment
  - Hash keys are precomputed in AST nodes
  - Resolving identifiers statically

- **Sequences:** You can build data processing or algorithmic pipelines using
  the functions and data structures from the sequence protocol. The first
  snippet in this readme is an example of that.

- **Pattern matching:** Gaya supports a limited form of pattern matching that
  is good for most purposes. For example:

  ```
  let xs = (1, 2, 3) in
  let (x, y, z) = xs in do
    assert(x == 1).
    assert(y == 2).
    assert(z == 3).
  end.
  ```

  Visit the documentation to learn more about it.

- **Hack pipes:** The language has Hack style pipes, which are pipes in which
  the result of the first expression can be piped into an arbitrary expression,
  not just function calls. To learn more, read the
  [documentation](#documentation).

#### Installing

```
git clone https://github.com/aloussase/gaya
cd gaya
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build
sudo cmake --install build
```

The `-DCMAKE_BUILD_TYPE=Release` bit is important. Otherwise the language is
much slower.

#### Documentation <a name="documentation" />

You can read the language documentation [here](./docs/SUMMARY.md).

For examples, take a look at the `examples` directory or at the standard
library under the `runtime` directory. You can also peek at the tests if you
want, though they may not use the latest features of the language.

Finally, you can take a look at [this
repo](https://github.com/aloussase/advent-of-code) where I'm solving Advent of
Code puzzles using Gaya.

#### Feedback & Contributing

Please feel free to open an issue or PR if you think you can make Gaya better!
The only prerequisites for contributing is getting familiar with codebase style
and using a code formatter that respects the project's configuration.

Areas in which you can help are:

- Open issues or PRs about confusing or uninformative diagnostics
- Open issues or PRs about segfaults (this is C++ after all hehe)
- Open issues or PRs to add something to the standard library

Of course, if something you thought about isn't on that list, feel free to
open an issue or PR about is anyway!

### Acknowledgements

- [zuiderkwast](https://github.com/zuiderkwast) for their [nanbox
  library](https://github.com/zuiderkwast/nanbox).
- [martinus](https://github.com/martinus) for their [awesome hash
  table](https://github.com/martinus/robin-hood-hashing).
- The Programming Languages discord community for all their help and support.
- Bob Nystrom for his book [Crafting
  Interpreters](http://craftinginterpreters.com/).

#### License

MIT
