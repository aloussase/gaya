# Sequences

```
include "sequences"
```

### `seq.next`

Return the next element in the sequence, or unit if there is none.

```
@param xs <sequence> The sequence from which to retrieve the next element.
```

### `seq.make`

Create a new user defined sequence from the provided callable.

The given callable should return the next element on each invocation, and
unit when it is done.

```
@param cb <function> The function to be invoked to retrieve the next element.
```

### `seq.foreach`

Call the provided function for each element in the sequence, discarding
the results.

```
@param s <sequence> A sequence.
@param f <function> A function to call on every element in the sequence.
```

### `seq.reduce`

Reduce the provided sequence according to the specified accumulator function,
using the given initial element. Return the initial element if the sequence
was empty.

```
@param s <sequence> A sequence.
@param init <object> The initial element.
@param f <function> The accumulator function.
```

### `seq.any`

Return true if any element in the provided sequence satisfies the given
predicate.

```
@param xs <sequence> The sequence on which to operate.
@param pred <function> A predicate.
```

### `seq.all`

Returns whether all the elements in the provided sequence satifsy
the given predicate.

```
@param xs <sequence> The sequence on which to operate.
@param pred <function> A predicate.
```

### `seq.max`

Return the greatest element in a sequence according to the '>' operator.

If the sequence is empty, unit is returned.

```
@param xs <sequence> The sequence.
```

### `seq.min`

Return the least element in a sequence according to the '<' operator.

If the sequence is empty, unit is returned.

```
@param xs <sequence> The sequence.
```

### `seq.minmax`

Return an array containing the least and greatest elements in a sequence
according to the '<' and '>' operators respectively.

If the sequence is empty, unit is returned.

```
@param xs <sequence> The sequence.
```

### `seq.map`

Return a new sequence that maps the given function over the elements of the
provided sequence.

```
@param xs <sequence> The sequence.
@param f <function> The transformation function.
```

### `seq.mapWithIndex`

Like map, but the callback function additionally receives the element index
as its first argument.

```
@param xs <sequence> The sequence.
@param func <function> The transformation function.
```

### `seq.filter`

Return a new sequence that filters out elements for which the provided
predicate function returns false.

```
@param xs <sequence> The sequence to filter.
@param pred <function> The predicate.
```

### `seq.zip`

Return a new sequence that applies the provided function to the elements of
the given sequence in a pairwise manner.

```
@param xs <sequence> The first sequence.
@parma ys <sequence> The second sequence.
```

### `seq.takewhile`

Return a sequence that takes elements from the input sequences as long as
they satisfy the given predicate.

```
@param xs <sequence> The sequence.
@param pred <function> The predicate.
```

### `seq.take`

Return a sequence that takes the first n elements of the provided one.

```
@param xs <sequence> The sequence.
@param n <number> The number of elements to take.
```

### `seq.dropwhile`

Return a sequence that drops elements from the input sequence as long as they
satisfy the given predicate.

```
@param xs <sequence> The sequence.
@param pred <function> The predicate.
```

### `seq.drop`

Return a new sequence that drops the first n elements of the provided one.

```
@param xs <sequence> The sequence.
@param n <number> The number of elements to drop.
```

### `seq.range`

Create a sequence that yields the numbers from start inclusive to end
exclusive.

```
@param start <number> The inclusive start of the range.
@param finish <number> The non-inclusive end of the range.
```

### `seq.enumerate`

Return a sequence that yields 2 element arrays where the first element is the
index of the second element from the provided sequence.

```
@param xs <sequence> The sequence to enumerate.
```

### `seq.toarray`

Return an array with the elements of the provided sequence.

```
@param xs <sequence> The sequence.
```

### `seq.tostring`

Return a string with the elements of the provided sequence.

`tostring` is called on every element of the sequence to transform it to a
string.

```
@param xs <sequence> The sequence.
```

### `seq.sum`

Return the sum of the numbers in the provided sequence.

This function consumes the provided sequence.

```
@param xs <sequence> The sequence.
```

### `seq.count`

Return the number of elements in a sequence.

This function consumes the provided sequence.

```
@param xs <sequence> The sequence.
```

### `seq.first`

Return the first element in the sequence, or unit if there are none.

```
@param xs <sequence> The sequence.
```
