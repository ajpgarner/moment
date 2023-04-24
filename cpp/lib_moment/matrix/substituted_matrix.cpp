/**
 * substituted_matrix.cpp
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "substituted_matrix.h"

#include <sstream>

namespace Moment {
    namespace {

        std::string make_description(const SubstitutedMatrix& sm, const SubstitutionList& sub_list) {
            std::stringstream ss;
            ss << "Monomial Substituted Matrix, Original: " << sm.source_matrix.description()
               << ",\n\t\tSubstitutions: ";
            sub_list.write_list(ss, ", ");

            return ss.str();
        }
    }

    SubstitutedMatrix::SubstitutedMatrix(const Context& context, SymbolTable& symbols,
                                         const MonomialMatrix& the_source,
                                         std::unique_ptr<SubstitutionList> subs)
         : MonomialMatrix{symbols, context, subs ? (*subs)(the_source.SymbolMatrix()) : nullptr},
           source_matrix{the_source}, sub_list{std::move(subs)} {
        assert(this->sub_list);

        this->set_description(make_description(*this, *sub_list));
    }

}