/**
 * full_correlator.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "collins_gisin.h"
#include "polynomial_tensor.h"
#include "symbolic/polynomial.h"

#include <string>

namespace Moment {

    namespace errors {
        class BadFCError : public std::runtime_error {
        public:
            explicit BadFCError(const std::string& what) : std::runtime_error(what) { }
        };
    };


    using FullCorrelatorElement = PolynomialElement;

    class FullCorrelator;

    using FullCorrelatorRange = TensorRange<FullCorrelator>;

    /**
     * Full correlator tensor.
     * Like probability tensor, but for binary measurement expectation values.
     */
    class FullCorrelator : public PolynomialTensor {
    public:
        struct TensorConstructInfo {
            AutoStorageIndex dimensions;
            std::vector<oper_name_t> operator_offset;
        };

    protected:
        std::vector<oper_name_t> operator_offset;

    public:
        FullCorrelator(const CollinsGisin& collinsGisin, const PolynomialFactory& factory,
                       TensorConstructInfo&& info,
                       TensorStorageType storage = TensorStorageType::Automatic);

        virtual ~FullCorrelator() noexcept = default;

        [[nodiscard]] std::string get_name(bool capital) const override {
            if (capital) {
                return "Full correlator tensor";
            } else {
                return "full correlator tensor";
            }
        }

    protected:
        [[nodiscard]] FullCorrelatorElement make_element_no_checks(IndexView index) const final;

    private:
        void calculate_correlators();

        [[nodiscard]] FullCorrelatorElement make_id() const;

        [[nodiscard]] FullCorrelatorElement make_one_party(size_t party, IndexView index) const;

        [[nodiscard]] FullCorrelatorElement
        make_two_party(size_t partyA, size_t partyB, IndexView index) const;

        [[nodiscard]] FullCorrelatorElement
        make_general(const SmallVector<size_t, 8>& involved_parties, IndexView index) const;

    };
}
