# The Holly Programming Language

The Holly Programming Language is a passion project, aiming to implement modern programming language features with a syntax that is both functional and pleasant to use.

## Syntax
Holly is whitespace and linebreak neutral, requiring semicolons after every statement.

### Comments
All Holly comments begin with `~`. Any text following a tilde until the end of the line will be treated as a comment

### Variables and Types
Holly requires every variable to begin with a prefix indicating type. Current primitives are:
- <u>Integers</u>: Prefixed with `i_`, stored internally as `int64_t`. 
- <u>Floats</u>: Prefixed with `f_`, stored internally as `double`
- <u>Booleans</u>: prefixed with `b_`, stored internally as `bool`
    - Unlike in C, booleans cannot be used in math operations, and do not evaluate to 1 or 0. 
- <u>Strings</u>: Prefixed with `s_`, stored internally as `char*`.
    - Strings can be concatenated with the `+` operator 
- <u>Void</u>: Used for functions with no return, prefixed with `v:`. It is not possible to assign variables with type `void`.


Variables are assigned using the `assign` keyword. They can be declared `final` on assignment, or set that way later using `finalize`. 

```
assign i_foo 7; ~declare an integer variable
final i_bar 8; ~declare a constant
i_foo = i_bar * 4; ~reassign a variable
i_foo -= 10; ~or do it in place
finalize i_foo; ~i_foo is now a constant
assign i_unset; ~can declare variables without setting them
i_unset = 5; ~and then set them later
```
### Control flow
Holly does not use `{braces}` for control flow. The syntax is instead closer to bash or other shell scripting languages.

#### Entry point
```
start;
~Code goes here
done;
```

#### If Statements:
```
if(true): 
    foo;
endif;

if(i_somevar >= 67):
    display "That number is big";
else:
    display "That number is small";
endif;
```
Note: Holly does not currently support any form of `elif` or `else if`.
#### While Statements
```
assign i_count 0;
assign i_adder 0;
while(i_count <= 10):
    i_adder += i_count;
    i_count += 1;
    if(i_count == 7):
        break;
    endif;
endwhile;
```

#### Loop Statements
Loop statements are intended for loops that should run x number of times. They function similar to `while` loops, except taking an `int` number of times to run the loop for instead of a condition to evaluate
```
final i_num_iters 200;
loop i_num_iters:
    display "apples";
endloop;
```

#### Functions
Holly requires functions to be prefixed in the same way as variables, however with a `:` following the prefix rather than a `_`.
```
fun b:is_odd(i_in)
    if(i_in % 2 == 0):
        return false;
    else:
        return true;
    endif;
endfun;

start;
display b:is_odd(3); ~displays 'true'
done;
```
#### Builtins
Holly supports the following builtins:
- `i:max(int, int)`
- `i:min(int, int)`
- `f:max(float, float)`
- `f:min(float, float)`
- `i:stringlen(string)`
- `f:itod(int)`

## Attributions

The Holly language incorporates [UThash](https://troydhanson.github.io/uthash/) for variable and function storage