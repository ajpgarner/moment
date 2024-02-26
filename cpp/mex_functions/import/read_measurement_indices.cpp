/**
 * read_measurement_indices.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "read_measurement_indices.h"

#include "utilities/read_as_scalar.h"
#include "utilities/reporting.h"
#include "utilities/visitor.h"

#include "scenarios/locality/locality_context.h"
#include "scenarios/inflation/inflation_context.h"

#include <sstream>
#include <stdexcept>

namespace Moment::mex {

    namespace {
        template<class RawIndexType>
        class IndexReaderVisitor {
        public:
            using return_type = std::vector<RawIndexType>;
            static constexpr const bool is_triplet = std::is_same_v<RawIndexTriplet, RawIndexType>;

        private:
            matlab::engine::MATLABEngine& matlabEngine;

        private:
            static std::string make_bad_index_string(size_t row, size_t col) {
                std::stringstream errSS;
                errSS << "Index " << col << " of row " << row << " should be a positive integer.";
                return errSS.str();
            }

        public:
            explicit IndexReaderVisitor(matlab::engine::MATLABEngine& matlabEngine) : matlabEngine{matlabEngine} { }

            /** Dense input -> dense monolithic output */
            template<std::convertible_to<size_t> data_t>
            return_type dense(const matlab::data::TypedArray<data_t> &matrix) {
                if (matrix.isEmpty()) {
                    return std::vector<RawIndexType>{};
                }

                std::vector<RawIndexType> output;
                const auto dims = matrix.getDimensions();
                assert(dims.size() == 2);
                assert((is_triplet && (dims[1] == 3)) || (!is_triplet && (dims[1] == 2)));

                for (size_t row = 0; row < dims[0]; ++row) {
                    if (matrix[row][0] < 1) {
                        throw BadParameter{make_bad_index_string(row+1, 1)};
                    }
                    if (matrix[row][1] < 1) {
                        throw BadParameter{make_bad_index_string(row+1, 2)};
                    }
                    if constexpr (is_triplet) {
                        if (matrix[row][2] < 1) {
                            throw BadParameter{make_bad_index_string(row+1, 2)};
                        }
                        output.emplace_back(static_cast<size_t>(matrix[row][0] - 1),
                                            static_cast<size_t>(matrix[row][1] - 1),
                                            static_cast<size_t>(matrix[row][2] - 1));
                    } else {
                        output.emplace_back(static_cast<size_t>(matrix[row][0] - 1),
                                            static_cast<size_t>(matrix[row][1] - 1));
                    }
                }
                return output;
            }

            /** Dense input -> dense monolithic output */
            return_type string(const matlab::data::StringArray& matrix) {
                if (matrix.isEmpty()) {
                    return std::vector<RawIndexType>{};
                }

                std::vector<RawIndexType> output;
                const auto dims = matrix.getDimensions();
                assert(dims.size() == 2);
                assert((is_triplet && (dims[1] == 3)) || (!is_triplet && (dims[1] == 2)));

                for (size_t row = 0; row < dims[0]; ++row) {
                    auto party_raw = read_positive_integer<size_t>(matlabEngine, "First index", matrix[row][0], 1);
                    auto mmt_raw = read_positive_integer<size_t>(matlabEngine, "Second index", matrix[row][1], 1);

                    if constexpr (is_triplet) {
                        auto outcome_raw = read_positive_integer<size_t>(matlabEngine, "Third index", matrix[row][2], 1);
                        // from matlab index to C++ index
                        output.emplace_back(static_cast<size_t>(party_raw - 1),
                                            static_cast<size_t>(mmt_raw - 1),
                                            static_cast<size_t>(outcome_raw - 1));
                    } else {
                        // from matlab index to C++ index
                        output.emplace_back(static_cast<size_t>(party_raw - 1),
                                            static_cast<size_t>(mmt_raw - 1), 0);
                    }
                }
                return output;
            }
        };

        static_assert(concepts::VisitorHasRealDense<IndexReaderVisitor<RawIndexPair>>);
        static_assert(concepts::VisitorHasString<IndexReaderVisitor<RawIndexPair>>);
        static_assert(!IndexReaderVisitor<RawIndexPair>::is_triplet);

        static_assert(concepts::VisitorHasRealDense<IndexReaderVisitor<RawIndexTriplet>>);
        static_assert(concepts::VisitorHasString<IndexReaderVisitor<RawIndexTriplet>>);
        static_assert(IndexReaderVisitor<RawIndexTriplet>::is_triplet);

        class bad_index_read : public std::runtime_error {
        public:
            explicit bad_index_read(const std::string& what) : std::runtime_error{what} { }
        };
    }

    std::vector<RawIndexPair>
    RawIndexPair::read_list(matlab::engine::MATLABEngine &matlabEngine, const matlab::data::Array &input) {
        return DispatchVisitor(matlabEngine, input, IndexReaderVisitor<RawIndexPair>{matlabEngine});
    }

    std::vector<RawIndexTriplet>
    RawIndexTriplet::read_list(matlab::engine::MATLABEngine &matlabEngine, const matlab::data::Array &input) {
        return DispatchVisitor(matlabEngine, input, IndexReaderVisitor<RawIndexTriplet>{matlabEngine});
    }


    std::pair<std::vector<RawIndexPair>, std::vector<RawIndexTriplet>>
    read_pairs_and_triplets(matlab::engine::MATLABEngine &matlabEngine, const matlab::data::Array &only_array) {
        if (only_array.getNumberOfElements() == 0) {
            return std::pair<std::vector<RawIndexPair>, std::vector<RawIndexTriplet>>();
        }
        auto onlyInputDims = only_array.getDimensions();
        if (onlyInputDims.size() != 2) {
            throw BadParameter{"Measurement/outcome list should be a 2D array."};
        }
        if (onlyInputDims[1] == 2) {
            return std::make_pair(RawIndexPair::read_list(matlabEngine, only_array),
                                  std::vector<RawIndexTriplet>{});

        } else if (onlyInputDims[1] == 3) {
            return std::make_pair(std::vector<RawIndexPair>{},
                                  RawIndexTriplet::read_list(matlabEngine, only_array));
        }
        throw BadParameter{"Measurement list should be a Nx2 array, outcome list a Nx3 array."};
    }

    std::pair<std::vector<RawIndexPair>, std::vector<RawIndexTriplet>>
    read_pairs_and_triplets(matlab::engine::MATLABEngine &matlabEngine, const matlab::data::Array &first_array,
                                 const matlab::data::Array &second_array) {

        if (first_array.getNumberOfElements() != 0) {
            auto inputOneDims = first_array.getDimensions();
            if ((inputOneDims.size() != 2) || (inputOneDims[1] != 2)) {
                throw BadParameter{"Measurement list should be a Nx2 array."};
            }
        }
        if (second_array.getNumberOfElements() != 0) {
            auto inputTwoDims = second_array.getDimensions();
            if ((inputTwoDims.size() != 2) || (inputTwoDims[1] != 3)) {
                throw BadParameter{"Outcome list should be a Nx3 array."};
            }
        }

        return std::make_pair(RawIndexPair::read_list(matlabEngine, first_array),
                              RawIndexTriplet::read_list(matlabEngine, second_array));
    }


    Locality::PMIndex PMConvertor::read_pm_index(const RawIndexPair &pair) {
        if (pair.first >= context.Parties.size()) {
            std::stringstream errSS;
            errSS << "Party #" << (pair.first+1) << " is out of range.";
            throw bad_index_read{errSS.str()};
        }

        const auto& party = context.Parties[pair.first];
        if (pair.second >= party.Measurements.size()) {
            std::stringstream errSS;
            errSS << "Measurement #" << (pair.second+1) << " is out of range.";
            throw bad_index_read{errSS.str()};
        }

        return Locality::PMIndex{this->context, static_cast<party_name_t>(pair.first),
                                                static_cast<mmt_name_t>(pair.second)};
    }

    Locality::PMOIndex PMConvertor::read_pmo_index(const RawIndexTriplet &triplet) {
        if (triplet.first >= context.Parties.size()) {
            std::stringstream errSS;
            errSS << "Party #" << (triplet.first+1) << " is out of range.";
            throw bad_index_read{errSS.str()};
        }

        const auto& party = context.Parties[triplet.first];
        if (triplet.second >= party.Measurements.size()) {
            std::stringstream errSS;
            errSS << "Measurement #" << (triplet.second+1) << " is out of range.";
            throw bad_index_read{errSS.str()};
        }

        const auto& mmt = party.Measurements[triplet.second];
        const size_t max_outcome_index = this->inclusive ? mmt.num_outcomes : mmt.num_operators();
        if (triplet.third >= max_outcome_index) {
            std::stringstream errSS;
            errSS << "Outcome #" << (triplet.third + 1) << " is out of range.";
            throw bad_index_read{errSS.str()};
        }


        return Locality::PMOIndex{this->context, static_cast<party_name_t>(triplet.first),
                                                 static_cast<mmt_name_t>(triplet.second),
                                                 static_cast<uint32_t>(triplet.third)};
    }

    std::vector<Locality::PMIndex> PMConvertor::read_pm_index_list(const std::span<const RawIndexPair> input) {
        size_t row_index = 1; // ML indexing
        std::vector<Locality::PMIndex> output{};
        output.reserve(input.size());
        for (const auto& entry : input) {
            try {
                output.emplace_back(this->read_pm_index(entry));
            } catch (const bad_index_read& bir) {
                std::stringstream errSS;
                errSS << "Error reading row " << row_index << ": " << bir.what();
                throw BadParameter{errSS.str()};
            }
            ++row_index;
        }
        return output;
    }

    std::vector<Locality::PMOIndex> PMConvertor::read_pmo_index_list(const std::span<const RawIndexTriplet> input) {
        size_t row_index = 1; // ML indexing
        std::vector<Locality::PMOIndex> output{};
        output.reserve(input.size());
        for (const auto& entry : input) {
            try {
                output.emplace_back(this->read_pmo_index(entry));
            } catch (const bad_index_read& bir) {
                std::stringstream errSS;
                errSS << "Error reading row " << row_index << ": " << bir.what();
                throw BadParameter{errSS.str()};
            }
            ++row_index;
        }
        return output;
    }

    Inflation::OVIndex OVConvertor::read_ov_index(const RawIndexPair &pair) {
        if (pair.first >= context.Observables().size()) {
            std::stringstream errSS;
            errSS << "Observable #" << (pair.first+1) << " is out of range.";
            throw bad_index_read{errSS.str()};
        }

        const auto& observable = context.Observables()[pair.first];
        if (pair.second >= observable.variant_count) {
            std::stringstream errSS;
            errSS << "Variant #" << (pair.second+1) << " is out of range.";
            throw bad_index_read{errSS.str()};
        }

        return Inflation::OVIndex{static_cast<oper_name_t>(pair.first), static_cast<oper_name_t>(pair.second)};
    }

    Inflation::OVOIndex OVConvertor::read_ovo_index(const RawIndexTriplet &triplet) {
        if (triplet.first >= context.Observables().size()) {
            std::stringstream errSS;
            errSS << "Observable #" << (triplet.first + 1) << " is out of range.";
            throw bad_index_read{errSS.str()};
        }

        const auto &observable = context.Observables()[triplet.first];
        if (triplet.second >= observable.variant_count) {
            std::stringstream errSS;
            errSS << "Variant #" << (triplet.second + 1) << " is out of range.";
            throw bad_index_read{errSS.str()};
        }

        if (observable.projective()) {
            const auto max_outcomes = this->inclusive ? observable.outcomes : (observable.outcomes-1);
            if (triplet.third > max_outcomes) {
                std::stringstream errSS;
                errSS << "Outcome #" << (triplet.third + 1) << " is out of range.";
                throw bad_index_read{errSS.str()};
            }
        } else {
            if (triplet.third != 0) {
                std::stringstream errSS;
                errSS << "Outcome #" << (triplet.third + 1) << " is out of range.";
                throw bad_index_read{errSS.str()};
            }
        }

        return Inflation::OVOIndex{static_cast<oper_name_t>(triplet.first),
                                   static_cast<oper_name_t>(triplet.second),
                                   static_cast<oper_name_t>(triplet.third)};
    }

    std::vector<Inflation::OVIndex> OVConvertor::read_ov_index_list(const std::span<const RawIndexPair> input) {
        size_t row_index = 1; // ML indexing
        std::vector<Inflation::OVIndex> output{};
        output.reserve(input.size());
        for (const auto& entry : input) {
            try {
                output.emplace_back(this->read_ov_index(entry));
            } catch (const bad_index_read& bir) {
                std::stringstream errSS;
                errSS << "Error reading row " << row_index << ": " << bir.what();
                throw BadParameter{errSS.str()};
            }
            ++row_index;
        }
        return output;
    }

    std::vector<Inflation::OVOIndex> OVConvertor::read_ovo_index_list(const std::span<const RawIndexTriplet> input) {
        size_t row_index = 1; // ML indexing
        std::vector<Inflation::OVOIndex> output{};
        output.reserve(input.size());
        for (const auto& entry : input) {
            try {
                output.emplace_back(this->read_ovo_index(entry));
            } catch (const bad_index_read& bir) {
                std::stringstream errSS;
                errSS << "Error reading row " << row_index << ": " << bir.what();
                throw BadParameter{errSS.str()};
            }
            ++row_index;
        }
        return output;
    }

}