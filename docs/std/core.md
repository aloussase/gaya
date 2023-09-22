# Core

### `typeof`

Return the type of an object as a string.
See src/object/typeof.cpp for the possible types.

```
@param o <object> The object for which to return the type.
```

### `assert`

Assert that something is truthy. If false, an intepreter error occurs with
the message that an assertion failed.

```
@param what <object> An object to assert.
```

### `tostring`

Return the string representation of the provided object.

```
@param o <object> The object for which to return the string representation.
```

### `issequence`

Return whether a given object can act as a sequence.

```
@param o <object> The object to test.
```

### `tosequence`

Convert the provided object into its sequence representation.

```
@param o <object> The object for which to return a sequence.
```
