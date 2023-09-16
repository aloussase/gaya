<p align="center">
  <h3>Gaya</h3>
</p>

```ocaml
"  Gaya is cool  "
    |> string.trim(_)
    |> string.concat(_, "!")
    |> string.split(_, " ")
```

Gaya is a scripting language I made for solving [AoC](https://adventofcode.com/)
problems in 2023. The name comes from my favourite cat.

Since its a toy language, its focus is on elegance above anything else. I
wanted something that makes me feel pleasure every time I use it. I hope this
way I can give pleasure to you too.

#### Features

- _Blazingly Fast (for a tree-walk interpreter)_: Gaya implements a number of
  optimizations, such as:

  - NaN Boxing
  - Uses an efficient hash table for the environment
  - Hash keys are precomputed in AST nodes

- Elegance: The language is mostly based on expressions. Every expression that
  is not used needs to be explicitely discarded. This may be tedious at first,
  but you'll come to appreciate it.

- Sequence protocol: You can build data processing or algorithmic pipelines
  using the functions and data structures conforming to the sequence protocol.
  The first snippet in this readme is an example of that.

- Hack pipes: The language has Hack style pipes, which are pipes in which the
  result of the first expression can be piped into any arbitrary expression,
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

You can read the language documentation [here](./docs/toc.md).

For examples, take a look at the `examples` directory of at the standard
library at `runtime/stdlib.gaya`.

### Feedback & Contributing

Please feel free to open an issue or PR if you think you can make Gaya better!
The only prerequisites for contributing is getting familiar with codebase style
and using a code formatter that respects the project's configuration.

#### License

MIT
