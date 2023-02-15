/**
 * export_substitution_list.cpp
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "export_substitution_list.h"


namespace Moment::mex {
    matlab::data::Array export_substitution_list(matlab::engine::MATLABEngine &engine,
                                                const std::map<symbol_name_t, double>& substitutions) {
        matlab::data::ArrayFactory factory;
        auto output = factory.createCellArray({1, substitutions.size()});

        size_t write_index = 0;
        for (auto [symbol, value] : substitutions) {
            auto pair = factory.createCellArray({1, 2});
            pair[0] = factory.createScalar(symbol);
            pair[1] = factory.createScalar(value);

            output[write_index] = std::move(pair);
            ++write_index;
        }
        return output;
    }
}

