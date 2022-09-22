/**
 * alphabetic_name.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "alphabetic_name.h"

#include "utilities/alphabetic_namer.h"

#include "utilities/read_as_scalar.h"
#include "utilities/reporting.h"
#include "utilities/visitor.h"

namespace NPATK::mex::functions {

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

            template<std::convertible_to<unsigned long> datatype>
            return_type dense(const matlab::data::TypedArray<datatype> &data) {
                matlab::data::ArrayFactory factory{};
                const auto& dimensions = data.getDimensions();
                auto output = factory.createArray<matlab::data::MATLABString>(dimensions);

                const size_t num_dim = dimensions.size();
                std::vector<size_t> indices(dimensions.size(), 0);

                auto read_iter = data.begin();
                auto write_iter = output.begin();

                while ((read_iter != data.end()) && (write_iter != output.end())) {
                    auto id = *read_iter;
                    if (!zero_index) {
                        if (id < 1) {
                            throw_error(this->the_engine, errors::bad_param,
                                        "Index 0 out of bounds. Did you mean to use 'zero_index' flag?");
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

    AlphabeticName::AlphabeticName(matlab::engine::MATLABEngine &matlabEngine, StorageManager& storage)
            : MexFunction(matlabEngine, storage, MEXEntryPointID::AlphabeticName, u"alphabetic_name") {
        this->min_outputs = 1;
        this->max_outputs = 1;

        this->flag_names.emplace(u"upper");
        this->flag_names.emplace(u"lower");

        this->flag_names.emplace(u"zero_index");

        this->mutex_params.add_mutex(u"upper", u"lower");

        this->min_inputs = 1;
        this->max_inputs = 1;
    }

    void AlphabeticName::operator()(IOArgumentRange output, std::unique_ptr<SortedInputs> inputPtr) {

        const auto& input = dynamic_cast<AlphabeticNameInputs&>(*inputPtr);

        AlphabeticNamer namer{input.is_upper};
        matlab::data::ArrayFactory factory{};

        if (input.inputs[0].getNumberOfElements() == 1) {
            auto id = read_as_uint64_or_fail(this->matlabEngine, input.inputs[0]);
            if (!input.zero_index) {
                if (id < 1) {
                    throw_error(this->matlabEngine, errors::bad_param,
                                "Index 0 out of bounds. Did you mean to use 'zero_index' flag?");
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

    std::unique_ptr<SortedInputs> AlphabeticName::transform_inputs(std::unique_ptr<SortedInputs> input) const {
        auto txInput = std::make_unique<AlphabeticNameInputs>(std::move(*input));

        // Check input type is parseable
        switch (txInput->inputs[0].getType()) {
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
                throw errors::BadInput{errors::bad_param, "Matrix type must be real numeric."};
        }


        return txInput;
    }

    AlphabeticNameInputs::AlphabeticNameInputs(SortedInputs &&input) : SortedInputs(std::move(input)) {
        this->is_upper = !this->flags.contains(u"lower");
        this->zero_index = this->flags.contains(u"zero_index");
    }
}