# Code Style.

> This is my code style for C and C++ projects.

## Functions.

### Naming.

`PascalCase`

### Declaration.

```cpp

i32 Add(i32 a, i32 b);

void V2Add(
    i32 * outX, i32 * outY,
    i32 x1,     i32 y1,
    i32 x2,     i32 y2
);

```

### Implementation.

```cpp

i32 
Add(i32 a, i32 b)
{

    return a + b;
}

```



