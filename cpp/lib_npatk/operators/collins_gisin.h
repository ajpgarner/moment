/**
 * collins_gisin.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "symbolic/symbol.h"
#include "operator_sequence.h"
#include "party.h"
#include "utilities/recursive_index.h"
#include "utilities/multi_dimensional_index_iterator.h"

#include <span>
#include <vector>

namespace NPATK {

    namespace errors {
        class cg_form_error : std::logic_error {
        public:
            explicit cg_form_error(const std::string& what) : std::logic_error(what) { }
        };
    }

    class MomentMatrix;

    /** Returns a list of explicit symbols, according to the parties and measurements chosen. */
    class CollinsGisinForm {
    public:
        using storage_t = std::vector<symbol_name_t>;

    public:
        const size_t Level;
    private:
        storage_t data;
        RecursiveDoubleIndex indices;

    public:
        CollinsGisinForm(const MomentMatrix& mm, size_t level);

        [[nodiscard]] std::span<const symbol_name_t> get_global(std::span<const size_t> mmtIndices) const;

        inline std::span<const symbol_name_t> get_global(std::initializer_list<size_t> mmtIndices) const {
            std::vector<size_t> v{mmtIndices};
            return get_global(v);
        }

    };
}