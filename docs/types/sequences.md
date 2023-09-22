# Sequence

Sequences are like lazy iterators in other programming languages. They can be
considered streams that compute their contents on demand. Sequences can be
created from a number of other objects using the `tosequence` builtin function.

```
include "sequences"

seq.range(1, 10000000).

seq.range(1, 10000000)
  |> seq.take(_, 3)
  |> seq.toarray(_)
  |> assert(_ == (1, 2, 3))
```

This creates a sequence over the numbers from 1 to 10000000, but doesn't
actually compute anything, so its instantaneous.

To get the next element in a sequence, use the `seq.next` function. This
function will return the successive elements of the sequence, or unit if the
sequence has been completely consumed.

```
xs :: tosequence("Gaya")

seq.next(xs). (* G *)
seq.next(xs). (* a *)
seq.next(xs). (* y *)
seq.next(xs). (* a *)
seq.next(xs). (* unit *)
```

Consult the documentation of each object to know if they support the sequence
protocol.

For operations available on sequences, see `runtime/sequences.gaya`.
