#include <gtest/gtest.h>

#include <cmath>
#include <limits>
#include <stdexcept>
#include <vector>

#include "Polynomial.h"
#include "PolynomialExceptions.h"

namespace
{
const double NaN = std::numeric_limits<double>::quiet_NaN();
const double Inf = std::numeric_limits<double>::infinity();

// Checks the class invariants.
void expectInvariants(const Polynomial& p)
{
    ASSERT_GE(p.size(), 1u);
    ASSERT_LE(p.size(), Polynomial::MAX_COEFFS);
    for (double v : p.coefficients())
    {
        EXPECT_TRUE(std::isfinite(v));
    }
    if (p.size() > 1)
    {
        EXPECT_NE(p.coefficients().back(), 0.0);
    }
}
}

// ---------- normal operation ----------

TEST(Construction, KeepsCoefficients)
{
    Polynomial p({1.0, 2.0, 3.0});
    EXPECT_EQ(p.size(), 3u);
    EXPECT_EQ(p.degree(), 2u);
    EXPECT_DOUBLE_EQ(p.at(1), 2.0);
    expectInvariants(p);
}

TEST(Construction, RemovesTrailingZeros)
{
    Polynomial p({1.0, 2.0, 0.0, 0.0});
    EXPECT_EQ(p.size(), 2u);
    expectInvariants(p);
}

TEST(Construction, ZeroPolynomialIsConstant)
{
    Polynomial p({0.0, 0.0});
    EXPECT_EQ(p.size(), 1u);
    EXPECT_DOUBLE_EQ(p.at(0), 0.0);
}

TEST(Operations, EvaluateHorner)
{
    Polynomial p({1.0, 2.0, 3.0});   // 1 + 2x + 3x^2
    EXPECT_NEAR(p.evaluate(2.0), 17.0, 1e-12);
    EXPECT_NEAR(p.evaluate(-1.0), 2.0, 1e-12);
    EXPECT_NEAR(p.evaluate(0.5), 2.75, 1e-12);
}

TEST(Operations, Addition)
{
    Polynomial r = add(Polynomial({1.0, 2.0}), Polynomial({3.0, 0.0, 4.0}));
    EXPECT_EQ(r.size(), 3u);
    EXPECT_NEAR(r.at(0), 4.0, 1e-12);
    EXPECT_NEAR(r.at(1), 2.0, 1e-12);
    EXPECT_NEAR(r.at(2), 4.0, 1e-12);
}

TEST(Operations, AdditionCancelsLeadingTerm)
{
    Polynomial r = add(Polynomial({1.0, 1.0}), Polynomial({1.0, -1.0}));
    EXPECT_EQ(r.size(), 1u);
    EXPECT_NEAR(r.at(0), 2.0, 1e-12);
}

TEST(Operations, Multiplication)
{
    Polynomial r = multiply(Polynomial({1.0, 1.0}), Polynomial({-1.0, 1.0}));   // x^2 - 1
    ASSERT_EQ(r.size(), 3u);
    EXPECT_NEAR(r.at(0), -1.0, 1e-12);
    EXPECT_NEAR(r.at(1), 0.0, 1e-12);
    EXPECT_NEAR(r.at(2), 1.0, 1e-12);
}

TEST(Operations, MultiplyAssignChangesPolynomial)
{
    Polynomial p({1.0, 1.0});
    p.multiplyAssign(Polynomial({1.0, 1.0}));   // (1 + x)^2
    ASSERT_EQ(p.size(), 3u);
    EXPECT_NEAR(p.at(1), 2.0, 1e-12);
    expectInvariants(p);
}

TEST(Operations, Derivative)
{
    Polynomial d = derivative(Polynomial({5.0, 3.0, 2.0, 1.0}));   // 3 + 4x + 3x^2
    ASSERT_EQ(d.size(), 3u);
    EXPECT_NEAR(d.at(0), 3.0, 1e-12);
    EXPECT_NEAR(d.at(1), 4.0, 1e-12);
    EXPECT_NEAR(d.at(2), 3.0, 1e-12);
}

TEST(Operations, SecondDerivativeAndZeroOrder)
{
    Polynomial p({0.0, 0.0, 0.0, 1.0});   // x^3
    Polynomial d2 = derivative(p, 2);      // 6x
    EXPECT_NEAR(d2.at(1), 6.0, 1e-12);
    Polynomial d0 = derivative(p, 0);
    EXPECT_EQ(d0.size(), p.size());
}

TEST(Operations, DerivativeOfConstantIsZero)
{
    Polynomial d = derivative(Polynomial({7.0}));
    EXPECT_EQ(d.size(), 1u);
    EXPECT_NEAR(d.at(0), 0.0, 1e-12);
}

TEST(Operations, IntegralOfQuadratic)
{
    EXPECT_NEAR(integral(Polynomial({0.0, 0.0, 3.0}), 0.0, 2.0), 8.0, 1e-12);   // 3x^2 -> x^3
    EXPECT_NEAR(integral(Polynomial({1.0}), 2.0, 5.0), 3.0, 1e-12);
}

// ---------- boundary values ----------

TEST(Boundary, ExactlyMaxCoefficientsAllowed)
{
    std::vector<double> c(Polynomial::MAX_COEFFS, 1.0);
    EXPECT_NO_THROW(Polynomial{c});
}

TEST(Boundary, LastValidIndex)
{
    Polynomial p({1.0, 2.0, 3.0});
    EXPECT_DOUBLE_EQ(p.at(p.size() - 1), 3.0);
}

TEST(Boundary, ProductWithMaxDegreeAllowed)
{
    // 500 + 501 - 1 = 1000 coefficients
    Polynomial a(std::vector<double>(500, 1.0));
    Polynomial b(std::vector<double>(501, 1.0));
    EXPECT_NO_THROW(multiply(a, b));
}

// ---------- exceptions ----------

TEST(Errors, EmptyCoefficients)
{
    EXPECT_THROW(Polynomial({}), EmptyCoefficientsException);
}

TEST(Errors, NaNCoefficient)
{
    EXPECT_THROW(Polynomial({1.0, NaN}), InvalidCoefficientException);
}

TEST(Errors, InfiniteCoefficientCarriesDiagnostics)
{
    try
    {
        Polynomial p({1.0, 2.0, Inf});
        FAIL() << "exception expected";
    }
    catch (const InvalidCoefficientException& ex)
    {
        EXPECT_EQ(ex.index(), 2u);
        EXPECT_TRUE(std::isinf(ex.value()));
    }
}

TEST(Errors, TooManyCoefficients)
{
    EXPECT_THROW(Polynomial(std::vector<double>(Polynomial::MAX_COEFFS + 1, 1.0)),
                 PolynomialOverflowException);
}

TEST(Errors, CoefficientIndexOutOfRange)
{
    Polynomial p({1.0, 2.0});
    EXPECT_THROW(p.at(2), std::out_of_range);
    EXPECT_THROW(p.at(1000), std::out_of_range);
    EXPECT_THROW(p.setCoefficient(2, 1.0), std::out_of_range);
}

TEST(Errors, MultiplicationDegreeOverflow)
{
    Polynomial a(std::vector<double>(600, 1.0));
    EXPECT_THROW(multiply(a, a), PolynomialOverflowException);
}

TEST(Errors, MultiplicationValueOverflowCarriesOperation)
{
    Polynomial a({1e200, 1.0});
    try
    {
        multiply(a, a);
        FAIL() << "exception expected";
    }
    catch (const PolynomialOverflowException& ex)
    {
        EXPECT_EQ(ex.operation(), "multiplication");
    }
}

TEST(Errors, AdditionOverflow)
{
    Polynomial a({1.7e308});
    EXPECT_THROW(add(a, a), PolynomialOverflowException);
}

TEST(Errors, InvalidOperationParameters)
{
    Polynomial p({1.0, 2.0});
    EXPECT_THROW(derivative(p, -1), std::invalid_argument);
    EXPECT_THROW(integral(p, NaN, 1.0), std::invalid_argument);
    EXPECT_THROW(integral(p, 0.0, Inf), std::invalid_argument);
    EXPECT_THROW(p.evaluate(NaN), std::invalid_argument);
}

TEST(Hierarchy, CatchableAsBaseClasses)
{
    EXPECT_THROW(Polynomial({}), PolynomialException);
    EXPECT_THROW(Polynomial({}), std::runtime_error);
    EXPECT_THROW(Polynomial({}), std::exception);
}

// ---------- invariants and strong guarantee ----------

TEST(Invariants, HoldAfterSuccessfulOperations)
{
    Polynomial p({1.0, 2.0, 3.0});
    p.setCoefficient(2, 0.0);   // leading term disappears
    expectInvariants(p);
    EXPECT_EQ(p.size(), 2u);
    p.multiplyAssign(Polynomial({0.0, 1.0}));
    expectInvariants(p);
}

TEST(StrongGuarantee, MultiplyAssignOverflowKeepsState)
{
    Polynomial p(std::vector<double>(600, 1.0));
    const Polynomial other(std::vector<double>(600, 1.0));
    const auto before = p.coefficients();
    EXPECT_THROW(p.multiplyAssign(other), PolynomialOverflowException);
    EXPECT_EQ(p.coefficients(), before);
}

TEST(StrongGuarantee, MultiplyAssignValueOverflowKeepsState)
{
    Polynomial p({1e200, 1.0});
    const Polynomial other({1e200, 1.0});
    const auto before = p.coefficients();
    EXPECT_THROW(p.multiplyAssign(other), PolynomialOverflowException);
    EXPECT_EQ(p.coefficients(), before);
}

TEST(StrongGuarantee, SetCoefficientNaNKeepsState)
{
    Polynomial p({1.0, 2.0, 3.0});
    const auto before = p.coefficients();
    EXPECT_THROW(p.setCoefficient(1, NaN), InvalidCoefficientException);
    EXPECT_EQ(p.coefficients(), before);
}

TEST(StrongGuarantee, SetCoefficientBadIndexKeepsState)
{
    Polynomial p({1.0, 2.0, 3.0});
    const auto before = p.coefficients();
    EXPECT_THROW(p.setCoefficient(5, 1.0), std::out_of_range);
    EXPECT_EQ(p.coefficients(), before);
}
