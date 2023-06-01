/**
 * polynomial.h
 * 
 * @copyright Copyright (c) 2022-2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "integer_types.h"

#include "monomial.h"

#include "utilities/small_vector.h"
#include "utilities/float_utils.h"

#include <algorithm>
#include <complex>
#include <iosfwd>
#include <map>
#include <utility>
#include <vector>

namespace Moment {
    class SymbolTable;

    class Polynomial {
    public:
        /**
         * Storage for linear  combination of symbolic expressions.
         * Monomial on stack, polynomial on heap.
         * */
        using storage_t = SmallVector<Monomial, 1>;

    private:
        storage_t data;

    public:

        Polynomial() = default;

        Polynomial(const Polynomial& rhs) = default;

        Polynomial(Polynomial&& rhs) = default;

        /** Construct combination from monomial */
        explicit Polynomial(const Monomial& monomial);

        /**
         * Construct combination from vector of monomials.
         * @tparam ordering_func_t Ordering functional on symbol expressions. Complex conjugates must be adjacent.
         * @param input The combination data.
         * @param order Instance of the ordering functional.
         */
        template<typename ordering_func_t = Monomial::IdLessComparator>
        explicit Polynomial(storage_t input,
                            const ordering_func_t& order = ordering_func_t{})
            : data{std::move(input)} {
            if (this->data.size() > 1) {
                this->sort(order);
                Polynomial::remove_duplicates(this->data);
            }
            Polynomial::remove_zeros(this->data);
        }

        /**
         * Construct combination from vector of monomials, and the symbol table.
         * @tparam ordering_func_t Ordering functional on symbol expressions. Complex conjugates must be adjacent.
         * @param input The combination data.
         * @param table The symbol table.
         * @param order Instance of the ordering functional.
         */
        template<typename ordering_func_t =  Monomial::IdLessComparator>
        explicit Polynomial(storage_t input,
                            const SymbolTable& table,
                            const ordering_func_t& order = ordering_func_t{})
                : data{std::move(input)} {
            this->fix_cc_in_place(table, false);
            if (this->data.size() > 1) {
                this->sort(order);
                Polynomial::remove_duplicates(this->data);
            }
            Polynomial::remove_zeros(this->data);
        }

        /** Construct combination from map of symbol names to weights.
         * This is automatically in id order, with no complex conjugates. */
        explicit Polynomial(const std::map<symbol_name_t, double>& input);

        inline Polynomial& operator=(const Polynomial& rhs) = default;

        inline Polynomial& operator=(Polynomial&& rhs) noexcept = default;

        Polynomial(std::initializer_list<Monomial> input)
            : Polynomial{storage_t{input}} { }

        [[nodiscard]] size_t size() const noexcept { return this->data.size(); }
        [[nodiscard]] bool empty() const noexcept { return this->data.empty(); }
        [[nodiscard]] auto begin() const noexcept { return this->data.cbegin(); }
        [[nodiscard]] auto end() const noexcept { return this->data.cend(); }
        [[nodiscard]] const Monomial& operator[](size_t index) const noexcept { return this->data[index]; }

        /** Set the expression to zero */
        void clear() noexcept { this->data.clear(); }

        /** Remove the last term from the expression */
        inline void pop_back() noexcept {
            this->data.pop_back();
        }

        /** Gets the last term from the expression */
        [[nodiscard]] inline const Monomial& back() const noexcept  {
            assert(!this->data.empty());
            return this->data.back();
        }

        /** True if combo consists of at most one element */
        [[nodiscard]] inline bool is_monomial() const noexcept {
            return this->data.size() <= 1;
        }

        /**
         * Downgrade combination to a single symbol expression.
         * @return The symbol expression.
         * @throws std::logic_error if Polynomial is not a monomial.
         */
        explicit operator Monomial() const;

        inline Polynomial& operator+=(const Polynomial& rhs) {
            return this->append(rhs);
        }

        [[nodiscard]] friend inline Polynomial operator+(const Polynomial& lhs, const Polynomial& rhs) {
            Polynomial output{lhs};
            output += rhs;
            return output;
        }

        Polynomial& operator*=(std::complex<double> factor) noexcept;

        [[nodiscard]] friend Polynomial operator*(Polynomial lhs, const std::complex<double> factor) noexcept {
            lhs *= factor;
            return lhs;
        }

        bool operator==(const Polynomial& rhs) const noexcept;

        inline bool operator!=(const Polynomial& rhs) const noexcept {
            return !(this->operator==(rhs));
        }

        /**
         * Replace all kX* with kX, if X is Hermitian, and kY* with -kY if Y is anti-Hermitian.
         * @return True, if this has changed the combination.
         */
        bool fix_cc_in_place(const SymbolTable& symbols, bool make_canonical = false) noexcept;

        /**
         * Return a new Polynomial with all Hermitian and anti-Hermitian operators in canonical format.
         * @see Polynomial::fix_cc_in_place
         */
        [[nodiscard]] Polynomial fix_cc(const SymbolTable& symbols, bool make_canonical = false) const {
            Polynomial output{*this};
            output.fix_cc_in_place(symbols, make_canonical);
            return output;
        }

        /**
         * Transform this combo into its complex conjugate.
         * @return True, if this might* have changed the combination. (*Some hermitian strings will trigger this).
         */
        bool conjugate_in_place(const SymbolTable& symbols) noexcept;

        /**
         * Return a new Polynomial equal to the complex conjugate of this one.
         */
        [[nodiscard]] Polynomial conjugate(const SymbolTable& symbols) const {
            Polynomial output{*this};
            output.conjugate_in_place(symbols);
            return output;
        }

        /** Put symbols into requested order */
        template<typename ordering_func_t = Monomial::IdLessComparator>
        inline void sort(const ordering_func_t& sort_func = ordering_func_t{}) {
            std::sort(this->data.begin(), this->data.end(), sort_func);
        }


        /**
         * Construct add symbols to this combo.
         * Undefined behaviour if ordering_func_t is different from that used to construct constituents.
         */
        template<typename ordering_func_t = Monomial::IdLessComparator>
        Polynomial& append(const Polynomial& rhs, const ordering_func_t& comp_less = ordering_func_t{}) {
            Polynomial& lhs = *this;

            // Debug validation
            assert(std::is_sorted(lhs.begin(), lhs.end(), comp_less));
            assert(std::is_sorted(rhs.begin(), rhs.end(), comp_less));

            // Get data iterators for RHS
            auto rhsIter = rhs.data.begin();
            const auto rhsEnd = rhs.data.end();

            // RHS is empty, nothing to do
            if (rhsIter == rhsEnd) {
                return *this;
            }

            // Get data iterators for LHS
            auto lhsIter = lhs.data.begin();
            const auto lhsEnd = lhs.data.end();

            // LHS is empty, copy RHS
            if (lhsIter == lhsEnd) {
                lhs.data.reserve(rhs.size());
                std::copy(rhs.data.cbegin(), rhs.data.cend(), std::back_inserter(lhs.data));
                return *this;
            }

            // Copy and merge, maintaining ID ordering
            storage_t output_data;
            while ((lhsIter != lhsEnd) || (rhsIter != rhsEnd)) {
                if ((rhsIter == rhsEnd) || ((lhsIter != lhsEnd) && comp_less(*lhsIter, *rhsIter))) {
                    output_data.push_back(*lhsIter); // Copy element from LHS
                    ++lhsIter;
                } else if ((lhsIter == lhsEnd) || ((rhsIter != rhsEnd) && comp_less(*rhsIter, *lhsIter))) {
                    output_data.push_back(*rhsIter); // Copy element from RHS
                    ++rhsIter;
                } else {
                    assert(lhsIter != lhsEnd);
                    assert(rhsIter != rhsEnd);
                    assert(lhsIter->id == rhsIter->id);
                    assert(lhsIter->conjugated == rhsIter->conjugated);

                    const auto sumVals = lhsIter->factor + rhsIter->factor;

                    if (!approximately_zero(sumVals)) {
                        output_data.emplace_back(lhsIter->id, sumVals, lhsIter->conjugated);
                    }
                    ++lhsIter;
                    ++rhsIter;
                }
            }
            this->data.swap(output_data);
            return *this;

        }


        /**
         * Get first included symbol ID.
         * Returns 0 if combo is zero.
         */
        [[nodiscard]] inline symbol_name_t first_id() const noexcept {
            if (this->empty()) {
                return 0;
            }
            return this->data[0].id;
        }

        /**
         * Get final included symbol ID.
         * Returns 0 if combo is zero.
         */
        [[nodiscard]] inline symbol_name_t last_id() const noexcept {
            if (this->empty()) {
                return 0;
            }
            return this->data.back().id;
        }

        /**
         * True if every factor in this symbol combo is real.
         */
        [[nodiscard]] bool real_factors() const noexcept {
            return std::none_of(this->data.begin(), this->data.end(), [](const auto& expr) {
                return expr.complex_factor();
            });
        }

        /**
         * True if sum of symbols is Hermitian.
         * @param symbols Symbol table (needed to know which symbols are purely real/imaginary etc.).
         */
        [[nodiscard]] bool is_hermitian(const SymbolTable& symbols) const noexcept;

        /**
         * True if other is conjugate of this symbol combo.
         * @param symbols Symbol table (needed to know which symbols are purely real).
         * @param other Polynomial to compare against.
         * @return True if this and other are Hermitian conjugates of each other.
         */
        [[nodiscard]] bool is_conjugate(const SymbolTable& symbols, const Polynomial& other) const noexcept;

        /**
         * Construct an empty combination.
         */
        inline static Polynomial Zero() {
            return Polynomial{};
        }

        /**
         * Construct a combination representing a scalar.
         * @param the_factor The scalar value (default: 1.0).
         */
        inline static Polynomial Scalar(const double the_factor = 1.0) {
            return Polynomial(storage_t{Monomial{1, the_factor , false}});
        }

        /**
         * Construct a combination representing a scalar.
         * @param the_factor Complex scalar value
         */
        inline static Polynomial Scalar(const std::complex<double> the_factor) {
            return Polynomial(storage_t{Monomial{1, the_factor, false}});
        }

        /**
         * Get a string expression of this Polynomial.
         */
        [[nodiscard]] std::string as_string() const;

        /**
         * Get a string expression of this Polynomial, as operators.
         * @param table The symbol table, with operator information.
         * @param show_braces True to add angular braces around operator sequences.
         * @return A newly constructed string representation of the polynomial.
         */
        [[nodiscard]] std::string as_string_with_operators(const SymbolTable& table, bool show_braces) const;

        /**
         * Get a string expression of this Polynomial, as operators.
         * @param os The output stream to write to.
         * @param table The symbol table, with operator information.
         * @param show_braces True to add angular braces around operator sequences.
         */
        void as_string_with_operators(std::ostream& os, const SymbolTable& table, bool show_braces) const;

        friend class PolynomialToBasisVec;
        friend class BasisVecToPolynomial;

        friend std::ostream& operator<<(std::ostream& os, const Polynomial& combo);

    private:
        static void remove_duplicates(Polynomial::storage_t &data);
        static void remove_zeros(Polynomial::storage_t &data);

    };


    /** Utility class for constructing symbol combos from data.
     * Allows for virtualization of sorting order template parameter. */
    class PolynomialFactory {
    public:
        const SymbolTable& symbols;

        explicit PolynomialFactory(const SymbolTable& symbols) : symbols{symbols} { }

        virtual ~PolynomialFactory() noexcept = default;

        [[nodiscard]] virtual Polynomial operator()(Polynomial::storage_t&& data) const {
            return Polynomial{std::move(data), symbols};
        }

        [[nodiscard]] virtual bool less(const Monomial& lhs, const Monomial& rhs) const {
            Monomial::IdLessComparator comparator;
            return comparator(lhs, rhs);
        }

        virtual void append(Polynomial& lhs, const Polynomial& rhs) const {
            lhs.append(rhs);
        }

        [[nodiscard]] Polynomial sum(const Polynomial& lhs, const Polynomial& rhs) const {
            Polynomial output{lhs};
            this->append(output, rhs); // <- virtual call.
            return output;
        }
    };

    // Only element is storage.
    static_assert(sizeof(Polynomial) == sizeof(Polynomial::storage_t));
}