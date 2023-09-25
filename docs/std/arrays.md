# Arrays

```
include "arrays"
```

### `array.length`

Return the length of an array.

```
@param a <array> The array.
```

### `array.isempty`

Return whether the given array is empty.

```
@param a <array> The array.
```

### `array.concat`

Append the elements of the second array to the end first one.

This mutates the first array.

```
@param a1 <array> The array onto which to append elements.
@param a2 <array> The array from which to extract elements.
```

### `array.push`

Append an element to the end of the array.

This mutates the original array.

```
@param a <array> The array onto which to push the element.
@param o <object> The element to append.
```

### `array.pop`

Remove and return the last element of the array.

This mutates the original array.

```
@param a <array> The array from which to remove the last element.
```

### `array.sort`

Sort the provided array in place.

The sorting is performed according to the provided comparison function.

```
@param a <array> The array to sort.
@param cmp <function> A comparison function.
```

### `array.range`

Return an array with the numbers from start to finish exclusive.

```
@param start <number> The start of the range.
@param finish <number> The non-inclusive end of the range. *)
```

### `array.join`

Return a string that is the result of joining the elements of the provided
array by the given separator.

```
@param a <array> The array to join.
@param sep <string> The separator to use
```
