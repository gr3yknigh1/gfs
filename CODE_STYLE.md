# Code Style.

> This is my code style for C and C++ projects.

## Functions.

### Naming.

`PascalCase`

### Declaration.

```c

S32 Add(S32 a, S32 b);

void V2Add(
    S32 * outX, S32 * outY,
    S32 x1,     S32 y1,
    S32 x2,     S32 y2
);

```

### Implementation.

```cpp

S32
Add(S32 a, S32 b)
{

    return a + b;
}

```



