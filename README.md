# Lab 7, variant 19: Polynomials. Exception handling and testing

C++20 project: static library PolynomialLib, demo program, antipattern program and Google Test executable.

## Structure

| Path | Purpose |
|------|---------|
| include/PolynomialExceptions.h | exception hierarchy |
| include/Polynomial.h, src/Polynomial.cpp | class Polynomial and operations (add, multiply, derivative, integral) |
| include/ScopeLogger.h | RAII class that prints constructor/destructor messages |
| demo/main.cpp | error scenarios, strong guarantee, stack unwinding |
| antipattern/main.cpp | throw 42 and throw std::string |
| tests/PolynomialTests.cpp | Google Test unit tests |

## Class invariants (Polynomial)

1. 1 <= size() <= 1000
2. all coefficients are finite (no NaN, no Inf)
3. the leading coefficient is not zero, except for the constant polynomial (trailing zeros are removed, degree() == size() - 1)

The invariants are checked in the constructor and in every modifying operation (setCoefficient, multiplyAssign).

## Exception hierarchy
```
std::exception
 +- std::runtime_error
 |   +- PolynomialException
 |       +- EmptyCoefficientsException
 |       +- InvalidCoefficientException   (index, value)
 |       +- PolynomialOverflowException   (operation, size)
 +- std::invalid_argument   (negative derivative order, NaN/Inf integration bounds, NaN x)
 +- std::out_of_range       (wrong coefficient index)
```

| Operation | Condition | Exception |
|-----------|-----------|-----------|
| constructor, setCoefficient | no coefficients | EmptyCoefficientsException |
| constructor, setCoefficient | NaN / Inf coefficient | InvalidCoefficientException |
| constructor | more than 1000 coefficients | PolynomialOverflowException |
| at, setCoefficient | index >= size | std::out_of_range |
| multiply, multiplyAssign | result has more than 1000 coefficients or a non-finite value | PolynomialOverflowException |
| add, derivative, integral | result is not finite | PolynomialOverflowException |
| derivative | negative order | std::invalid_argument |
| integral | NaN / Inf bound | std::invalid_argument |
| evaluate | NaN argument | std::invalid_argument |

Exceptions are caught by const&, the most specific handler first.

## Exception safety guarantees

| Method | Guarantee |
|--------|-----------|
| constructor | strong (data are validated in a local vector) |
| setCoefficient | strong: copy, change, validate, then swap (noexcept) |
| multiplyAssign | strong: the product is computed in a temporary polynomial, then swap (noexcept) |
| size, degree, coefficients | no-throw (noexcept) |
| at, evaluate, add, multiply, derivative, integral | do not change the object |

Destructors do not throw (all memory is owned by std::vector).

## Expected output of stack unwinding (written BEFORE running)

Call chain: main -> level1 -> level2 -> level3, each function has a local ScopeLogger.
level3 multiplies a polynomial of 600 coefficients by itself (1199 coefficients > 1000) and gets PolynomialOverflowException, level2 catches it as PolynomialException, prints a diagnostic and rethrows with throw;, main handles it.
The handler of main is outside the try block where ScopeLogger("main") lives, so the logger of main is destroyed before the handler runs.
```
[ctor] main
[ctor] level1
[ctor] level2
[ctor] level3
[dtor] level3
level2: caught "...", rethrowing
[dtor] level2
[dtor] level1
[dtor] main
main: PolynomialOverflowException: ...
```

## Antipattern: throwing int and std::string

antipattern/main.cpp throws 42 and std::string. catch (const std::exception&) does not catch them, only catch (...) does, and it has no access to the type or the value.

Conclusion: standard and custom exceptions (derived from std::exception) have a common base, a what() message, a type for selecting the handler and can carry diagnostic data. Primitive types give none of this, are not caught by std::exception handlers and may leave the error unhandled, so they must not be thrown.

## Build and run (Windows, Visual Studio, vcpkg)

In "x64 Native Tools Command Prompt for VS":
```
set VCPKG_ROOT=C:\vcpkg
cmake --preset default
cmake --build build --config Release
build\Release\PolynomialDemo.exe
build\Release\AntipatternDemo.exe
build\Release\PolynomialTests.exe
ctest --test-dir build -C Release
```

vcpkg.json declares the gtest dependency, vcpkg installs it automatically during cmake --preset default.

## Actual output

```
[ctor] main
[ctor] level1
[ctor] level2
[ctor] level3
[dtor] level3
level2: caught "Overflow in multiplication (size 1199)", rethrowing
[dtor] level2
[dtor] level1
[dtor] main
main: PolynomialOverflowException: Overflow in multiplication (size 1199)
```

The actual order matches the expected one. Destructors run in reverse order of construction during stack unwinding, so no resources are leaked.
