/**
 * read_opseq_polynomial.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include <complex>
#include <optional>
#include <string>

#include "symbolic/polynomial.h"

#include "MatlabDataArray.hpp"


namespace matlab::engine {
    class MATLABEngine;
}

namespace Moment {
    class Context;
    class SymbolTable;
    class RawPolynomial;
}

namespace Moment::mex {

    class StagingMonomial;

    class StagingPolynomial {
    private:
        matlab::engine::MATLABEngine& matlabEngine;

        std::string name;

    public:
        explicit StagingPolynomial(matlab::engine::MATLABEngine& engine,
                                   const matlab::data::Array& input,
                                   std::string inputName);

        ~StagingPolynomial() noexcept;

        /**
         * Resolve numeric strings of operator numbers into a contextualized (i.e. simplified) operator sequence.
         * @param context The context to interpret the operator sequences.
         * @throws matlab::Exception (errors::bad_param) if operator string is invalid (e.g. out of range).
         */
        void supply_context(const Context& context);

        /**
         * Instantiates a raw polynomial from contextualized inputs
         */
        RawPolynomial to_raw_polynomial() const;

        /**
         * Look up symbols for contextualized monomials.
         * @param symbols The table of registered symbols.
         * @param fail_quietly If true, missing symbols are set to -1.
         * @throws matlab::Exception (errors::bad_param) if fail_quietly is false and a symbol cannot be found.
         * @returns True if all symbols are found.
         */
        bool find_symbols(const SymbolTable& symbol_table, bool fail_quietly = false);

        /**
         * Looks up symbols for contextualized monomials, or creates new symbols if they don't already exist.
         * Can be called safely after find_symbols.
         * A write lock should be held.
         * @param symbols The table of registered symbols.
         */
        void find_or_register_symbols(SymbolTable& symbols);

        /**
         * Instantiate a polynomial from resolved symbols.
         * @param factory The polynomial factory.
         */
        [[nodiscard]] Polynomial to_polynomial(const PolynomialFactory& factory) const;

        /**
         * True if polynomial can be instantiated
         */
         [[nodiscard]] inline bool ready() const noexcept {
             return this->symbols_resolved;
         }

         /**
          * True if polynomial contains aliases
          */
         [[nodiscard]] inline bool any_aliases() const noexcept {
             return this->aliases_found;
         }

    private:
        std::unique_ptr<StagingMonomial[]> data;

        size_t data_length = 0;

        bool symbols_resolved = false;

        bool aliases_found = false;
    };



}
