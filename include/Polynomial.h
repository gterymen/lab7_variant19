#pragma once

#include <cstddef>
#include <vector>

// Polynomial with real coefficients, coefficient index = power of x.
//
// Class invariants (hold for every accessible object):
//   1. 1 <= size() <= MAX_COEFFS
//   2. all coefficients are finite numbers
//   3. the leading coefficient is not zero, except for the constant polynomial
//      (trailing zeros are removed, so degree() == size() - 1)
class Polynomial
{
public:
    static constexpr std::size_t MAX_COEFFS = 1000;

    // Throws: EmptyCoefficientsException, InvalidCoefficientException,
    //         PolynomialOverflowException (more than MAX_COEFFS coefficients).
    explicit Polynomial(std::vector<double> coefficients);

    std::size_t size() const noexcept;
    std::size_t degree() const noexcept;
    const std::vector<double>& coefficients() const noexcept;

    // Throws std::out_of_range if index >= size().
    double at(std::size_t index) const;

    // Throws std::invalid_argument if x is NaN.
    double evaluate(double x) const;

    // Sets one coefficient. Safety guarantee: STRONG (copy-and-swap).
    // Throws: std::out_of_range, InvalidCoefficientException.
    void setCoefficient(std::size_t index, double value);

    // this = this * other. Safety guarantee: STRONG: the product is computed in a
    // temporary polynomial, the state is changed only by noexcept swap.
    // Throws PolynomialOverflowException.
    void multiplyAssign(const Polynomial& other);

private:
    std::vector<double> c_;
};

// Throws PolynomialOverflowException if the result is not representable.
Polynomial add(const Polynomial& a, const Polynomial& b);
Polynomial multiply(const Polynomial& a, const Polynomial& b);

// Derivative of the given order. Throws std::invalid_argument if order < 0.
Polynomial derivative(const Polynomial& a, int order = 1);

// Definite integral. Throws std::invalid_argument if a bound is not finite.
double integral(const Polynomial& a, double lower, double upper);
