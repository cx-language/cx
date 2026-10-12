# Vector math

Fixed-size arrays support element-wise arithmetic: `float[3]`
can be used directly as a 3D vector.

## Element-wise arithmetic

Both operands must have the same size; element types must match exactly
(use explicit conversions and float literals for float arrays).

```cs
void main() {
    float[3] a = [1.0, 2.0, 3.0];
    float[3] b = [4.0, 5.0, 6.0];

    float[3] c = a + b; // [5.0, 7.0, 9.0]
    float[3] d = a * 2.0; // [2.0, 4.0, 6.0], scalar broadcasts
    float[3] e = 10.0 - b; // [6.0, 5.0, 4.0], scalar on either side

    println(c[0]); // prints 5
    println(d[2]); // prints 6
    println(e[0]); // prints 6
}
```

Supported element-wise ops: `+ - * / %` (and `%%`), bitwise `& | ^ << >>`
for integer elements, and `== !=` (which return `bool`: `==` is true when
all elements are equal, `!=` when any differ). Unary `-` negates each
element. Unlike scalar arithmetic, integer element-wise ops wrap on
overflow instead of aborting.

```cs
void main() {
    float[3] a = [1.0, 2.0, 3.0];
    float[3] b = [1.0, 2.0, 3.0];
    println(a == b); // prints true
    println(a == [1.0, 2.0, 4.0]); // prints false
}
```

Sizes must match; mismatched sizes are a compile error. Only arrays of
known (constant) size support element-wise ops; generic sizes (`T[N]` with
symbolic `N`) are rejected with a clear error.

<details>
<summary>Vector codegen</summary>

Element-wise ops compile directly to SIMD vector instructions on the LLVM
backend, in every build mode; larger arrays use a loop over vector chunks
with a scalar tail. The C backend emits a scalar loop for the C compiler
to vectorize.

</details>

## Vector operations

Arrays of any size double as vectors: `dot()` returns the dot product,
`length()` the Euclidean length, and `normalized()` a unit-length copy
(the receiver is left unchanged). `normalized()` is only meaningful for
floating-point elements. The free function `cross()` returns the cross
product of two 3-element arrays.

```cs
void main() {
    float[3] a = [1.0, 2.0, 3.0];
    float[3] b = [4.0, 5.0, 6.0];

    println(a.dot(b)); // prints 32
    println(a.length()); // prints 3.7416573867739413

    float[3] n = a.normalized();
    println(n[0]); // prints 0.26726124

    float[3] c = cross(a, b);
    println(c[0]); // prints -3
    println(c[1]); // prints 6
    println(c[2]); // prints -3
}
```

## Swizzles

Arrays of up to 4 numeric elements support GLSL-style swizzles:
1–4 characters from `xyzw`, `rgba`, or `stpq` (one set per swizzle),
mapping to indices 0–3 (`x`/`r`/`s` → 0, `y`/`g`/`t` → 1, `z`/`b`/`p` → 2,
`w`/`a`/`q` → 3). A single character returns the element; multiple
characters return a new array. Out-of-bounds swizzles are compile errors.

Swizzles are writable: assigning to a single character writes the element,
and assigning an array to multiple characters writes each element in order.

<details>
<summary>Swizzle write restrictions</summary>

Only direct assignment writes back (indexing or re-swizzling a
multi-character swizzle is a compile error).

</details>

```cs
void main() {
    float[3] v = [1.0, 2.0, 3.0];
    v.x = 10.0;
    println(v[0]); // prints 10

    v.yz = [20.0, 30.0];
    println(v[1]); // prints 20
    println(v[2]); // prints 30
}
```

```cs
void main() {
    float[3] v = [1.0, 2.0, 3.0];
    println(v.x); // prints 1, same as v[0]
    println(v.z); // prints 3

    float[2] xy = v.xy; // [1.0, 2.0]
    float[3] zyx = v.zyx; // [3.0, 2.0, 1.0]
    float[2] rg = v.rg; // [1.0, 2.0], rgba set works too
    println(xy[0]); // prints 1
    println(zyx[0]); // prints 3
    println(rg[1]); // prints 2

    float[4] w = [1.0, 2.0, 3.0, 4.0];
    println(w.w); // prints 4
    float[2] ra = w.ra; // [1.0, 4.0]
    println(ra[1]); // prints 4
}
```

See [Arrays](arrays) for array basics.
