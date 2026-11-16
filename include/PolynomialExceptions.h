#pragma once

#include <cstddef>
#include <stdexcept>
#include <string>

// Base class of all exceptions of the library.
class PolynomialException : public std::runtime_error
{
public:
    explicit PolynomialException(const std::string& message)
        : std::runtime_error(message)
    {
    }
};

// The set of coefficients is empty.
class EmptyCoefficientsException : public PolynomialException
{
public:
    EmptyCoefficientsException()
        : PolynomialException("Coefficient set is empty")
    {
    }
};

// A coefficient is NaN or infinite.
class InvalidCoefficientException : public PolynomialException
{
private:
    std::size_t index_;
    double value_;

public:
    InvalidCoefficientException(std::size_t index, double value)
        : PolynomialException("Invalid coefficient: c[" + std::to_string(index)
                              + "] is not finite"),
          index_(index),
          value_(value)
    {
    }

    std::size_t index() const noexcept { return index_; }
    double value() const noexcept { return value_; }
};

// A result is not representable: too many coefficients or a non-finite value.
class PolynomialOverflowException : public PolynomialException
{
private:
    std::string operation_;
    std::size_t size_;

public:
    PolynomialOverflowException(const std::string& operation, std::size_t size)
        : PolynomialException("Overflow in " + operation + " (size " + std::to_string(size) + ")"),
          operation_(operation),
          size_(size)
    {
    }

    const std::string& operation() const noexcept { return operation_; }
    std::size_t size() const noexcept { return size_; }
};
