/**
 * polynomial_factory.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "polynomial.h"

#include "symbol_errors.h"

#include <iosfwd>

namespace Moment {

    class SymbolTable;
    class RawPolynomial;

    namespace errors {

    }

    /** Utility class for constructing symbol combos from data.
     * Allows for virtualization of sorting order template parameter. */
    class PolynomialFactory {
    public:
        const SymbolTable& symbols;

        /** If a value is less than zero_tolerance * eps, treat it as zero. */
        const double zero_tolerance = 1.0;

        explicit PolynomialFactory(const SymbolTable& symbols, double zero_tolerance = 1.0)
                : symbols{symbols}, zero_tolerance{zero_tolerance} { }

        virtual ~PolynomialFactory() noexcept = default;

        /**
         * Construct a Polynomial using the factory settings.
         * @param data The data to synthesize into a polynomial.
         * @return Newly constructed Polynomial.
         */
        [[nodiscard]] virtual Polynomial operator()(Polynomial::storage_t&& data) const = 0;

        /**
         * Construct a Polynomial from a RawPolynomial object
         * @param raw Operator sequences paired with complex coefficients.
         * @return A newly constructed Polynomial.
         * @throws errors::unregistered_operator_sequence If any entry does not correspond to something in SymbolTable.
         */
        [[nodiscard]] Polynomial construct(const RawPolynomial& raw) const;

        /**
         * Construct a Polynomial from a RawPolynomial object
         * @param symbols Write-accessible symbol table, where new symbols will be registered.
         * @param raw Operator sequences paired with complex coefficients.
         * @return A newly constructed Polynomial.
         */
        [[nodiscard]] Polynomial register_and_construct(SymbolTable& symbols, const RawPolynomial& raw) const;

        SmallVector<size_t, 1> presort_data(Polynomial::storage_t& data) const;

        [[nodiscard]] virtual bool less(const Monomial& lhs, const Monomial& rhs) const = 0;

        /**
         * Gets the maximum degree of a polynomial.
         */
        [[nodiscard]] virtual size_t maximum_degree(const Polynomial& poly) const;

        virtual void append(Polynomial& lhs, const Polynomial& rhs) const = 0;

        /** Copies polynomial up to scaling factor */
        [[nodiscard]] inline Polynomial scale(const Polynomial& lhs, std::complex<double> factor) const {
            Polynomial copy{lhs};
            copy.scale(factor, this->zero_tolerance);
            return copy;
        }

        /**
         * Efficiently combine LHS and RHS to make a polynomial with 0, 1 or 2 elements.
         */
        [[nodiscard]] Polynomial sum(const Monomial& lhs, const Monomial& rhs) const;

        /**
         * Adds a monomial to an (already ordered) Polynomial
         * * @complexity O(N+1)
         */
        [[nodiscard]] Polynomial sum(const Polynomial& lhs, const Monomial& rhs) const;

        /**
         * Adds to (already sorted) Polynomials to make a new Polynomial.
         * @complexity O(N1+N2)
         */
        [[nodiscard]] Polynomial sum(const Polynomial& lhs, const Polynomial& rhs) const;

        [[nodiscard]] inline bool is_hermitian(const Polynomial& poly) const {
            return poly.is_hermitian(this->symbols, this->zero_tolerance);
        }

        [[nodiscard]] inline bool is_antihermitian(const Polynomial& poly) const {
            return poly.is_antihermitian(this->symbols, this->zero_tolerance);
        }

        /**
         * Encodes Monomial into lexicographic order, such that key(A) < key(B) iff less(A<B).
         */
        [[nodiscard]] virtual std::pair<uint64_t, uint64_t> key(const Monomial& mono) const noexcept = 0;

        /** Gets string name of polynomial factory */
        [[nodiscard]] virtual const std::string& name() const = 0;

        /** Outputs string description of polynomial factory */
        friend std::ostream& operator<<(std::ostream& os, const PolynomialFactory& factory);
    };


    template<typename comparator_t, typename get_name_struct_t>
    class PolynomialFactoryImpl : public PolynomialFactory {
    public:
        using Comparator = comparator_t;

    protected:
        Comparator comparator;
        std::string func_name;

    public:

        template<typename... Args>
        explicit PolynomialFactoryImpl(const SymbolTable& symbols, double zero_tolerance = 1.0, Args&&... args)
                : PolynomialFactory{symbols, zero_tolerance}, comparator{std::forward<Args>(args)...} {
            this->func_name = get_name_struct_t::name;
        }

        [[nodiscard]] Polynomial operator()(Polynomial::storage_t &&data) const override {

            return Polynomial{std::move(data), this->symbols,  this->comparator, this->zero_tolerance};
        }

        [[nodiscard]] bool less(const Monomial &lhs, const Monomial &rhs) const override {
            return comparator(lhs, rhs);
        }

        void append(Polynomial &lhs, const Polynomial &rhs) const override {
            lhs.append(rhs, this->comparator, this->zero_tolerance);
        }

        [[nodiscard]] const std::string& name() const override {
            return this->func_name;
        }

        [[nodiscard]] std::pair<uint64_t, uint64_t> key(const Monomial& mono) const noexcept override {
            return this->comparator.key(mono);
        }
    };

    struct ByIDPolynomialFactory_Name {
        constexpr static char name[] = "Sort by ID";
    };

    using ByIDPolynomialFactory = PolynomialFactoryImpl<IdLessComparator, ByIDPolynomialFactory_Name>;

}