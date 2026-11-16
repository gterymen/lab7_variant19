#include "Polynomial.h"

#include <algorithm>
#include <cmath>
#include <string>
#include <utility>
#include <stdexcept>

#include "PolynomialExceptions.h"

namespace
{
// Removes trailing zeros (the constant polynomial keeps one coefficient).
void trim(std::vector<double>& c)
{
    while (c.size() > 1 && c.back() == 0.0)
    {
        c.pop_back();
    }
}

// Validates the coefficients and brings them to the canonical form.
void normalize(std::vector<double>& c)
{
    if (c.empty())
    {
        throw EmptyCoefficientsException();
    }
    if (c.size() > Polynomial::MAX_COEFFS)
    {
        throw PolynomialOverflowException("construction", c.size());
    }
    for (std::size_t i = 0; i < c.size(); ++i)
    {
        if (!std::isfinite(c[i]))
        {
            throw InvalidCoefficientException(i, c[i]);
        }
    }
    trim(c);
}

// Builds a polynomial from computed coefficients; a non-finite value means overflow.
Polynomial fromComputed(std::vector<double> c, const char* operation)
{
    if (c.size() > Polynomial::MAX_COEFFS)
    {
        throw PolynomialOverflowException(operation, c.size());
    }
    for (double v : c)
    {
        if (!std::isfinite(v))
        {
            throw PolynomialOverflowException(operation, c.size());
        }
    }
    return Polynomial(std::move(c));
}
}

Polynomial::Polynomial(std::vector<double> coefficients)
{
    normalize(coefficients);
    c_.swap(coefficients);
}

std::size_t Polynomial::size() const noexcept
{
    return c_.size();
}

std::size_t Polynomial::degree() const noexcept
{
    return c_.size() - 1;
}

const std::vector<double>& Polynomial::coefficients() const noexcept
{
    return c_;
}

double Polynomial::at(std::size_t index) const
{
    if (index >= c_.size())
    {
        throw std::out_of_range("coefficient index " + std::to_string(index) + " is out of range");
    }
    return c_[index];
}

double Polynomial::evaluate(double x) const
{
    if (std::isnan(x))
    {
        throw std::invalid_argument("x must not be NaN");
    }
    double result = 0.0;
    for (std::size_t i = c_.size(); i-- > 0;)
    {
        result = result * x + c_[i];
    }
    return result;
}

// Safety guarantee: strong
void Polynomial::setCoefficient(std::size_t index, double value)
{
    if (index >= c_.size())
    {
        throw std::out_of_range("coefficient index " + std::to_string(index) + " is out of range");
    }
    std::vector<double> copy = c_;
    copy[index] = value;
    normalize(copy);   // may throw, the object is not modified yet
    c_.swap(copy);     // noexcept commit
}

// Safety guarantee: strong
void Polynomial::multiplyAssign(const Polynomial& other)
{
    Polynomial product = multiply(*this, other);   // may throw, the object is not modified yet
    c_.swap(product.c_);                           // noexcept commit
}

Polynomial add(const Polynomial& a, const Polynomial& b)
{
    const auto& ca = a.coefficients();
    const auto& cb = b.coefficients();
    std::vector<double> r(std::max(ca.size(), cb.size()), 0.0);
    for (std::size_t i = 0; i < ca.size(); ++i)
    {
        r[i] += ca[i];
    }
    for (std::size_t i = 0; i < cb.size(); ++i)
    {
        r[i] += cb[i];
    }
    return fromComputed(std::move(r), "addition");
}

Polynomial multiply(const Polynomial& a, const Polynomial& b)
{
    const auto& ca = a.coefficients();
    const auto& cb = b.coefficients();
    const std::size_t n = ca.size() + cb.size() - 1;
    if (n > Polynomial::MAX_COEFFS)
    {
        throw PolynomialOverflowException("multiplication", n);
    }
    std::vector<double> r(n, 0.0);
    for (std::size_t i = 0; i < ca.size(); ++i)
    {
        for (std::size_t j = 0; j < cb.size(); ++j)
        {
            r[i + j] += ca[i] * cb[j];
        }
    }
    return fromComputed(std::move(r), "multiplication");
}

Polynomial derivative(const Polynomial& a, int order)
{
    if (order < 0)
    {
        throw std::invalid_argument("derivative order must not be negative");
    }
    std::vector<double> r = a.coefficients();
    for (int k = 0; k < order && !(r.size() == 1 && r[0] == 0.0); ++k)
    {
        if (r.size() == 1)
        {
            r[0] = 0.0;
            break;
        }
        std::vector<double> d(r.size() - 1);
        for (std::size_t i = 1; i < r.size(); ++i)
        {
            d[i - 1] = r[i] * static_cast<double>(i);
        }
        r = std::move(d);
    }
    return fromComputed(std::move(r), "derivative");
}

double integral(const Polynomial& a, double lower, double upper)
{
    if (!std::isfinite(lower) || !std::isfinite(upper))
    {
        throw std::invalid_argument("integration bounds must be finite");
    }
    const auto& c = a.coefficients();
    double result = 0.0;
    for (std::size_t i = 0; i < c.size(); ++i)
    {
        const double p = static_cast<double>(i + 1);
        result += c[i] / p * (std::pow(upper, p) - std::pow(lower, p));
    }
    if (!std::isfinite(result))
    {
        throw PolynomialOverflowException("integral", c.size());
    }
    return result;
}
