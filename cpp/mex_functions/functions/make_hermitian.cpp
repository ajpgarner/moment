/**
 * make_hermitian.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "make_hermitian.h"

#include "symbolic/symbol_set.h"
#include "symbolic/symbol_tree.h"

#include "fragments/export_substitution_list.h"
#include "fragments/export_symbol_properties.h"
#include "fragments/substitute_elements_using_tree.h"
#include "fragments/read_symbol_or_fail.h"
#include "utilities/reporting.h"
#include "utilities/visitor.h"


namespace NPATK::mex::functions  {



    MakeHermitian::MakeHermitian(matlab::engine::MATLABEngine &matlabEngine)
            : MexFunction(matlabEngine, MEXEntryPointID::MakeHermitian, u"make_hermitian") {
        this->min_outputs = 1;
        this->max_outputs = 3;
        this->min_inputs = 1;
        this->max_inputs = 1;
    }

    std::pair<bool, std::basic_string<char16_t>> MakeHermitian::validate_inputs(const SortedInputs &input) const {
        // Should be guaranteed~
        assert(!input.inputs.empty());

        auto inputDims = input.inputs[0].getDimensions();
        if (inputDims.size() != 2) {
            return {false, u"Input must be a matrix."};
        }

        if (inputDims[0] != inputDims[1]) {
            return {false, u"Input must be a square matrix."};
        }

        switch(input.inputs[0].getType()) {
            case matlab::data::ArrayType::MATLAB_STRING:
                break;
            default:
                return {false, u"Matrix type must be of strings."};
        }

        return {true, u""};
    }


    namespace {
        /**
         * Read through matlab dense numerical matrix, and identify pairs of elements that are not symmetric.
         */
        class NonhermitianElementIdentifierVisitor {
            matlab::engine::MATLABEngine& engine;

        public:
            using return_type = NPATK::SymbolSet;

        public:
            explicit NonhermitianElementIdentifierVisitor(matlab::engine::MATLABEngine &the_engine)
                    : engine(the_engine) { }

            /**
              * Read through matlab dense numerical matrix, and identify pairs of elements that are not symmetric.
              * @param data The data array
              * @return A vector of non-matching elements, in canonical form.
              */
            return_type string(const matlab::data::StringArray &data) {
                SymbolSet output{};

                const size_t dimension = data.getDimensions()[0];
                for (size_t i = 0; i < dimension; ++i) {
                    // Register diagonal element as real symbol:
                    NPATK::SymbolExpression diag{read_symbol_or_fail(this->engine, data, i, i)};
                    output.add_or_merge(Symbol{diag.id, false});

                    for (size_t j = i + 1; j < dimension; ++j) {
                        NPATK::SymbolExpression upper{read_symbol_or_fail(this->engine, data, i, j)};
                        NPATK::SymbolExpression lower{read_symbol_or_fail(this->engine, data, j, i)};
                        lower.conjugated = !lower.conjugated;

                        if (upper != lower) {
                            output.add_or_merge(SymbolPair{upper, lower});
                        } else {
                            output.add_or_merge(Symbol{upper.id});
                            output.add_or_merge(Symbol{lower.id});
                        }
                    }
                }

                return output;
            }
        };

        static_assert(concepts::VisitorHasString<NonhermitianElementIdentifierVisitor>);

        /**
         * Read through matlab dense numerical matrix, and identify pairs of elements that are not hermitian.
         * @param engine Reference to matlab engine
         * @param data The data array
         * @return A SymbolSet of elements in the matrix, with raw inferred equalities.
         */
        SymbolSet identify_nonhermitian_elements(matlab::engine::MATLABEngine &engine,
                                                 const matlab::data::Array &data) {
            return DispatchVisitor(engine, data, NonhermitianElementIdentifierVisitor{engine});
        }
    }

    void MakeHermitian::operator()(FlagArgumentRange outputs, SortedInputs&& inputs) {
        auto unique_constraints = identify_nonhermitian_elements(matlabEngine, inputs.inputs[0]);

        if (verbose) {
            std::stringstream ss2;
            ss2 << "\nFound " << unique_constraints.symbol_count() << " symbols and "
                << unique_constraints.link_count() << " links.\n";
            if (debug) {
                ss2 << "Sorted, unique constraints:\n"
                    << unique_constraints;
            }
            print_to_console(matlabEngine, ss2.str());
        }

        unique_constraints.pack();
        auto symbol_tree = SymbolTree{std::move(unique_constraints)};

        if (debug) {
            std::stringstream ss3;
            ss3 << "\nTree, initial:\n" << symbol_tree;
            NPATK::mex::print_to_console(matlabEngine, ss3.str());
        }

        symbol_tree.simplify();

        if (verbose) {
            std::stringstream ss4;
            ss4 << "\nTree, simplified:\n" << symbol_tree << "\n";
            NPATK::mex::print_to_console(matlabEngine, ss4.str());
        }

        if (outputs.size() >= 1) {
            outputs[0] = NPATK::mex::make_hermitian_using_tree(matlabEngine, inputs.inputs[0], symbol_tree);
        }

        if (outputs.size() >= 2) {
            outputs[1] = NPATK::mex::export_substitution_list(matlabEngine, symbol_tree);
        }

        if (outputs.size() >= 3) {
            outputs[2] = NPATK::mex::export_symbol_properties(matlabEngine, symbol_tree);
        }

    }
}