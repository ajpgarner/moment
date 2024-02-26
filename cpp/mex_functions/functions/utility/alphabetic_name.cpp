/**
 * alphabetic_name.cpp
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "alphabetic_name.h"

#include "utilities/alphabetic_namer.h"

#include "utilities/read_as_scalar.h"
#include "utilities/reporting.h"
#include "utilities/visitor.h"

namespace Moment::mex::functions {
    namespace {
        class AlphabeticNamerMatrixVisitor {
        private:
            matlab::engine::MATLABEngine &the_engine;
            const AlphabeticNamer& namer;
            bool zero_index = false;

        public:
            AlphabeticNamerMatrixVisitor(matlab::engine::MATLABEngine &matlabEngine,
                                         const AlphabeticNamer& namer, bool zero_index)
                                         : the_engine{matlabEngine}, namer{namer}, zero_index{zero_index} { }

            using return_type = matlab::data::TypedArray<matlab::data::MATLABString>;

            template<std::convertible_to<size_t> datatype>
            return_type dense(const matlab::data::TypedArray<datatype> &data) {
                matlab::data::ArrayFactory factory{};
                const auto& dimensions = data.getDimensions();
                auto output = factory.createArray<matlab::data::MATLABString>(dimensions);

                const size_t num_dim = dimensions.size();
                std::vector<size_t> indices(dimensions.size(), 0);

                auto read_iter = data.begin();
                auto write_iter = output.begin();

                while ((read_iter != data.end()) && (write_iter != output.end())) {
                    auto id = static_cast<size_t>(*read_iter); // NOLINT(cert-str34-c)
                    if (!zero_index) {
                        if (id < 1) {
                            throw BadParameter{"Index 0 out of bounds. Did you mean to use 'zero_index' flag?"};
                        }
                        id -= 1;
                    }
                    *write_iter = this->namer(id);

                    ++read_iter;
                    ++write_iter;
                }
                return output;
            }
        };
    }

    AlphabeticNameParams::AlphabeticNameParams(SortedInputs &&input)
            : SortedInputs(std::move(input)) {
        this->is_upper = !this->flags.contains(u"lower");
        this->zero_index = this->flags.contains(u"zero_index");

        // Check input type is parseable
        switch (this->inputs[0].getType()) {
            case matlab::data::ArrayType::SINGLE:
            case matlab::data::ArrayType::DOUBLE:
            case matlab::data::ArrayType::INT8:
            case matlab::data::ArrayType::UINT8:
            case matlab::data::ArrayType::INT16:
            case matlab::data::ArrayType::UINT16:
            case matlab::data::ArrayType::INT32:
            case matlab::data::ArrayType::UINT32:
            case matlab::data::ArrayType::INT64:
            case matlab::data::ArrayType::UINT64:
                break;
            default:
                throw BadParameter{"Matrix type must be real numeric."};
        }
    }

    AlphabeticName::AlphabeticName(matlab::engine::MATLABEngine &matlabEngine, StorageManager& storage)
            : ParameterizedMTKFunction{matlabEngine, storage} {
        this->min_outputs = 1;
        this->max_outputs = 1;

        this->flag_names.emplace(u"upper");
        this->flag_names.emplace(u"lower");

        this->flag_names.emplace(u"zero_index");

        this->mutex_params.add_mutex(u"upper", u"lower");

        this->min_inputs = 1;
        this->max_inputs = 1;
    }

    void AlphabeticName::operator()(IOArgumentRange output, AlphabeticNameParams &input) {
        AlphabeticNamer namer{input.is_upper};
        matlab::data::ArrayFactory factory{};

        if (input.inputs[0].getNumberOfElements() == 1) {
            auto id = read_positive_integer<uint64_t>(this->matlabEngine, "Input", input.inputs[0]);
            if (!input.zero_index) {
                if (id < 1) {
                    throw BadParameter{"Index 0 out of bounds. Did you mean to use 'zero_index' flag?"};
                }
                --id;
            }
            std::string suggestedName = namer(id);
            output[0] = factory.createCharArray(suggestedName);
        } else {
            output[0] = DispatchVisitor(this->matlabEngine, input.inputs[0],
                            AlphabeticNamerMatrixVisitor{this->matlabEngine, namer, input.zero_index});
        }
    }
}