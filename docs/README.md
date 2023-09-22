# Gaya

Gaya is a dynamically typed scripting language that uses a tree-walk
interpreter for its implementation and performs a number of optimizations to
make it usable. Some of these optimizations are:

- Nan Boxing
- Efficient hash table implementation
- Precomputing hash keys in AST nodes
- Resolving identifiers statically

These optimizations are important because I wanted to use
the language for (Advent of Code)[https://adventofcode.com/]
and without them it was ridiculously slow.

## Motivation

I've had interest in programming languages for a long time, but never before
had I put a somewhat serious effort into implementing my own.

This effort started when I learnt about [Typst](https://typst.app/) and tried
to use it to solve some graph processing problems for a hackathon. I realized
that a language as simple as it was enough for the task at hand, and thought to
myself: "Surely I can make one myself!". And here we are now.

I made the language with a focus on elegance first and performance second (you
can't ask for much from a tree-walk interpreter). I wanted to make something
that makes me feel pleasure every time I use it. I hope this way I can give
pleasure to you too.
