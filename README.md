# The Holly Programming Language

The Holly Programming Language is a passion project, aiming to implement modern programming language features with a syntax that is both functional and pleasant to use.

## Syntax
Holly is whitespace and linebreak neutral, requiring semicolons after every statement.

### Variables
Holly requires every variable to begin with a prefix indicating type. Current primitives are:
- <u>Integers</u>: Prefixed with `i_`, stored internally as `int64_t`. 
- <u>Floats</u>: Prefixed with `f_`, stored internally as `double`
- <u>Booleans</u>: prefixed with `b_`, stored internally as `bool`
    - Note: Unlike in C, booleans cannot be used in math operations, and do not evaluate to 1 or 0. 


Variables are assigned using the `assign` keyword. They can be declared `final` on assignment, or set that way later using `finalize`. 

```
assign i_foo 7; ~declare an integer variable
final i_bar 8; ~declare a constant
i_foo = i_bar * 4; ~reassign a variable
i_foo -= 10; ~or do it in place
finalize i_foo; ~i_foo is now a constant
```


## Attributions

The Holly language incorporates [UThash](https://troydhanson.github.io/uthash/) for variable storage and lookup