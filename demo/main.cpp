#include <exception>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <vector>

#include "Polynomial.h"
#include "PolynomialExceptions.h"
#include "ScopeLogger.h"

namespace
{
void level3()
{
    ScopeLogger log("level3");
    // degree 599 * degree 599 = 1199 coefficients > MAX_COEFFS
    Polynomial big(std::vector<double>(600, 1.0));
    big.multiplyAssign(big);
    std::cout << "this line is never reached\n";
}

void level2()
{
    ScopeLogger log("level2");
    try
    {
        level3();
    }
    catch (const PolynomialException& ex)
    {
        std::cout << "level2: caught \"" << ex.what() << "\", rethrowing\n";
        throw;
    }
}

void level1()
{
    ScopeLogger log("level1");
    level2();
}

template <typename F>
void scenario(const char* title, F action)
{
    std::cout << "\n=== " << title << " ===\n";
    try
    {
        action();
    }
    catch (const EmptyCoefficientsException& ex)
    {
        std::cout << "EmptyCoefficientsException: " << ex.what() << '\n';
    }
    catch (const InvalidCoefficientException& ex)
    {
        std::cout << "InvalidCoefficientException: " << ex.what() << " (index=" << ex.index() << ")\n";
    }
    catch (const PolynomialOverflowException& ex)
    {
        std::cout << "PolynomialOverflowException: " << ex.what() << " (operation=" << ex.operation()
                  << ")\n";
    }
    catch (const PolynomialException& ex)
    {
        std::cout << "PolynomialException: " << ex.what() << '\n';
    }
    catch (const std::out_of_range& ex)
    {
        std::cout << "std::out_of_range: " << ex.what() << '\n';
    }
    catch (const std::invalid_argument& ex)
    {
        std::cout << "std::invalid_argument: " << ex.what() << '\n';
    }
}
}

int main()
{
    const double nan = std::numeric_limits<double>::quiet_NaN();
    const double inf = std::numeric_limits<double>::infinity();

    std::cout << "##### Part 1: error scenarios #####\n";
    Polynomial p({1.0, 2.0, 3.0});   // 1 + 2x + 3x^2
    std::cout << "p(2) = " << p.evaluate(2.0) << ", degree = " << p.degree() << '\n';

    scenario("1. empty coefficient set", [] { Polynomial q({}); });
    scenario("2. NaN coefficient", [&] { Polynomial q({1.0, nan}); });
    scenario("3. Inf coefficient", [&] { Polynomial q({inf, 1.0}); });
    scenario("4. wrong coefficient index", [&] { p.at(10); });
    scenario("5. overflow in multiplication (too high degree)", [] {
        Polynomial a(std::vector<double>(600, 1.0));
        multiply(a, a);
    });
    scenario("6. overflow in multiplication (value too large)", [] {
        Polynomial a({1e200, 1.0});
        multiply(a, a);
    });
    scenario("7. wrong operation parameter (negative derivative order)", [&] { derivative(p, -1); });
    scenario("8. wrong operation parameter (NaN integration bound)", [&] { integral(p, nan, 1.0); });

    std::cout << "\n##### Part 2: strong guarantee #####\n";
    Polynomial a(std::vector<double>(600, 1.0));
    std::cout << "size before multiplyAssign: " << a.size() << '\n';
    scenario("multiplyAssign with overflow", [&] { a.multiplyAssign(a); });
    std::cout << "size after failed multiplyAssign: " << a.size() << " (unchanged)\n";
    Polynomial b({1.0, 1.0});   // 1 + x
    b.multiplyAssign(Polynomial({-1.0, 1.0}));   // (1 + x)(x - 1) = x^2 - 1
    std::cout << "(1 + x)(x - 1): c0 = " << b.at(0) << ", c1 = " << b.at(1) << ", c2 = " << b.at(2) << '\n';
    scenario("setCoefficient with NaN", [&] { b.setCoefficient(1, nan); });
    std::cout << "c1 after failed setCoefficient: " << b.at(1) << " (unchanged)\n";
std::cout << "\n##### Part 3: stack unwinding #####\n";
    try
    {
        ScopeLogger log("main");
        level1();
    }
    catch (const PolynomialOverflowException& ex)
    {
        std::cout << "main: PolynomialOverflowException: " << ex.what() << '\n';
    }
    catch (const PolynomialException& ex)
    {
        std::cout << "main: PolynomialException: " << ex.what() << '\n';
    }
    catch (const std::exception& ex)
    {
        std::cout << "main: std::exception: " << ex.what() << '\n';
    }
    return 0;
}
