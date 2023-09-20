/**
 * polynomial_tensor.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "polynomial_tensor.h"

#include "symbolic/polynomial_factory.h"

#include "utilities/format_factor.h"

#include <iostream>


namespace Moment {

    PolynomialTensor::PolynomialTensor(const CollinsGisin &collinsGisin, const PolynomialFactory &factory,
                                       AutoStorageIndex&& dimensions, TensorStorageType storage)
        : AutoStorageTensor<PolynomialElement, poly_tensor_explicit_element_limit>(std::move(dimensions), storage),
      collinsGisin{collinsGisin}, symbolPolynomialFactory{factory}, missingSymbols{std::nullopt}  {
        if (this->StorageType == TensorStorageType::Explicit) {
            this->missingSymbols.emplace(this->ElementCount);
        }
    }

    bool PolynomialTensor::fill_missing_polynomials() {
        if (this->hasAllSymbols) {
            return true;
        }
        assert(this->StorageType == TensorStorageType::Explicit);
        assert(this->missingSymbols.has_value());

        this->hasAllSymbols = true;
        DynamicBitset<uint64_t, size_t> still_missing(this->ElementCount);
        for (size_t symbol_id : *this->missingSymbols) {
            // Attempt to resolve.
            const bool found = this->attempt_symbol_resolution(this->data[symbol_id]);
            if (!found) {
                this->hasAllSymbols = false;
                still_missing.set(symbol_id);
            }
        }

        if (!this->hasAllSymbols) {
            this->missingSymbols->swap(still_missing);
        }

        return this->hasAllSymbols;
    }


    bool PolynomialTensor::attempt_symbol_resolution(PolynomialElement& element) const {
        Polynomial::storage_t poly_data;
        for (auto& mono_elem : element.cgPolynomial) {
            assert(mono_elem.id > 0);
            auto cg_view = this->collinsGisin.elem_no_checks(static_cast<size_t>(mono_elem.id) - 1);
            if (cg_view->symbol_id >= 0) {
                poly_data.emplace_back(cg_view->symbol_id, mono_elem.factor);
            } else {
                return false;
            }
        }

        element.symbolPolynomial = this->symbolPolynomialFactory(std::move(poly_data));
        element.hasSymbolPoly = true;
        return true;
    }


    Polynomial PolynomialTensor::CGPolynomial(const AutoStorageIndexView indices) const {
        this->validate_index(indices);
        if (this->StorageType == TensorStorageType::Explicit) {
            return this->data[this->index_to_offset_no_checks(indices)].cgPolynomial;
        } else {
            return this->make_element_no_checks(indices).cgPolynomial;
        }
    }


    std::string PolynomialTensor::elem_as_string(const PolynomialElement &element) const {
        std::stringstream ss;
        this->elem_as_string(ss, element);
        return ss.str();
    }

    void PolynomialTensor::elem_as_string(std::ostream& os, const PolynomialElement &element) const {

        // Firstly, if we have symbolic poly, wrap in contextual OS and delegate.
        if (element.hasSymbolPoly) {
            ContextualOS cSS{os, this->collinsGisin.context, this->collinsGisin.symbol_table};
            cSS.format_info.display_symbolic_as = ContextualOS::DisplayAs::Operators;
            cSS.format_info.show_braces = true;
            cSS << element.symbolPolynomial;
            return;
        }

        // Empty string is always just 0.
        if (element.cgPolynomial.empty()) {
            os << "0";
            return;
        }

        bool done_once = false;
        for (const auto& elem : element.cgPolynomial) {
            // Zero
            if ((elem.id == 0) || (approximately_zero(elem.factor))) {
                if (done_once) {
                    os << " + ";
                }
                os << "0";
                done_once = true;
                continue;
            }

            // Is element a scalar?
            const bool is_scalar = (elem.id == 1);

            // Write factor
            const bool need_space = format_factor(os, elem.factor, is_scalar, done_once);
            done_once = true;

            // Scalar, factor alone is enough
            if (is_scalar) {
                continue;
            }

            if (need_space) {
                os << " ";
            }

            // Get CG entry
            auto cg_entry = this->collinsGisin.elem_no_checks(elem.id - 1);

            // Get context-formatted sequence
            os << "<" << cg_entry->sequence.formatted_string() << ">";
            done_once = true;
        }
    }


}