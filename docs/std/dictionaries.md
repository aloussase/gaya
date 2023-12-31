# Dictionaries

```
include "dictionaries"
```

### `dict.length`

Return the number of items in the provided dictionary.

```
@param dict <dictionary> The dictionary.
```

### `dict.set`

Insert a key-value pair in the provided dictionary.

This mutates the original dictionary.

```
@param dict <dictionary> The dictionary in which to do the insertion.
@param key <object> The key.
@param value <object> The value.
@return The provided dictionary.
```

### `dict.remove`

Remove the specified key-value pair from the dictionary.

This mutates the original dictionary.

```
@param dict <dictionary> The dictionary from which to remove the pair.
@param key <object> The key to remove.
@return The removed value.
```

### `dict.contains`

Returns a truthy value if the specified key has a corresponding value in the
provided dictionary, unit otherwise.

```
@param dict <dictionary> The dictionary to test.
@param key <object> The key of interest.
```

### `dict.keys`

Return a sequence over the keys of the provided dictionary.

### `dict.values`

Return a sequence over the values of the provided dictionary.

### `dict.setdefault`

Set the value for a given key in the provided dictionary, or a default value
if the key is not found.

The new value if computed from the old value by applying to it a
transformation function.

```
@param dict <dictionary> The dictionary.
@param key <object> The key.
@param valueFunc <function> The function to compute the new value.
@param default <object> The default value in case the key is not found.
```
