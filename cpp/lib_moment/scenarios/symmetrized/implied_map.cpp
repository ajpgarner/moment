/**
 * implied_map.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "implied_map.h"

#include "map_core.h"
#include "representation.h"
#include "symmetrized_matrix_system.h"

#include "scenarios/context.h"

#include "symbolic/symbol_table.h"
#include "utilities/dynamic_bitset.h"

#include <limits>
#include <sstream>

namespace Moment::Symmetrized {

    namespace {
        constexpr bool is_close(const double x, const double y, const double eps_mult = 1.0) {
            return (std::abs(x - y) < std::numeric_limits<double>::epsilon() * std::max(std::abs(x), std::abs(y)));
        }

    }


    ImpliedMap::ImpliedMap(SymmetrizedMatrixSystem& sms,
                           const Representation& rep)

       : origin_symbols{sms.base_system().Symbols()}, target_symbols{sms.Symbols()}, max_length{rep.word_length} {
        assert(&origin_symbols != &target_symbols);

        // Check length of origin symbols is sufficiently long:
        const size_t osg_length = origin_symbols.OSGIndex.max_length();
        if (rep.word_length > osg_length) {
            std::stringstream errSS;
            errSS << "Symbol only guarantees dictionary up to length " << osg_length
                  << " but representation of length " << rep.word_length << " was supplied.";
            throw errors::bad_map{errSS.str()};
        }


        // TODO: Pass OSG and some-way of mapping OSG indices to symbol table indices.

        // Get number of elements


        // First, build constants
        this->map.assign(origin_symbols.size(), SymbolCombo::Zero()); // 0 -> 0 always
        this->map[0] = SymbolCombo::Zero();      // 0 -> 0 always.
        this->map[1] = SymbolCombo::Scalar(1.0); // 1 -> 1 always.

        // Get group average and check first column maps 1 -> 1
        const repmat_t raw_remap = rep.sum_of() / static_cast<double>(rep.size()); // = average * rep.size() - to avoid division.

        // Extract non-trivial subcomponent
        this->nontrivial_core = std::make_unique<MapCore>(this->origin_symbols, raw_remap);

        // Create scalar remap:
        for (auto [row_id, scalar] : this->nontrivial_core->constants) {
            auto [symbol_id, conjugated] = origin_symbols.OSGIndex(row_id);
            assert(!conjugated);
            this->map[symbol_id] = SymbolCombo::Scalar(scalar / static_cast<double>(rep.size()));
        }

        //this->process_nontrivial_core();
    }

    ImpliedMap::~ImpliedMap() noexcept = default;


    const SymbolCombo& ImpliedMap::operator()(symbol_name_t symbol_id) const {
        // Bounds check
        if ((symbol_id < 0) || (symbol_id >= this->map.size())) {
            std::stringstream ss;
            ss << "Symbol " << symbol_id << " not defined in implied map.";
            throw errors::bad_map{ss.str()};
        }

        // Remap
        return this->map[symbol_id];
    }

    SymbolCombo ImpliedMap::operator()(const SymbolExpression &symbol) const {
        // Get raw combo, or throw range error
        SymbolCombo output = (*this)(symbol.id);

        // Apply transformations (using target symbol table)
        output *= symbol.factor;
        if (symbol.conjugated) {
            output.conjugate_in_place(this->target_symbols);
        }

        return output;
    }
}