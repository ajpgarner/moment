/**
 * substituted_matrix.cpp
 * 
 * Copyright (c) 2023 Austrian Academy of Sciences
 */
#include "substituted_matrix.h"

#include <sstream>

namespace Moment {

    SubstitutedMatrix::SubstitutedMatrix(const Context& context, SymbolTable& symbols,
                                         const SymbolicMatrix& the_source,
                                         std::unique_ptr<SubstitutionList> subs)
         : SymbolicMatrix{context, symbols, subs ? (*subs)(the_source.SymbolMatrix()) : nullptr},
            source_matrix{the_source}, sub_list{std::move(subs)} {
        assert(this->sub_list);
    }

    std::string SubstitutedMatrix::description() const {
        std::stringstream ss;
        ss << "Substituted Matrix, Original: " << this->source_matrix.description()
        << ",\n\t\tSubstitutions: ";
        this->sub_list->write_list(ss, ", ");

        return ss.str();
    }
}