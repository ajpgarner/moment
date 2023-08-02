/**
 * contextual_os_tests.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "gtest/gtest.h"

#include "scenarios/context.h"
#include "scenarios/contextual_os.h"
#include "scenarios/contextual_os_helper.h"

#include "scenarios/algebraic/algebraic_context.h"
#include "scenarios/algebraic/algebraic_matrix_system.h"

#include <memory>
#include <sstream>


namespace Moment::Tests {

    namespace {
        class AcceptsCOSObject {
        private:
            int value = 0;
        public:
            explicit AcceptsCOSObject(int value) : value{value} { }

            friend auto& operator<<(ContextualOS& cos, const AcceptsCOSObject& obj) {
                cos.os << cos.context.size() << ":" << obj.value;
                return cos;
            }
        };

        class RejectsCOSObject {
        private:
            int value = 0;
        public:
            explicit RejectsCOSObject(int value) : value{value} { }

            friend auto& operator<<(std::ostream& os, const RejectsCOSObject& obj) {
                os << obj.value;
                return os;
            }
        };
    }


    TEST(Scenarios_ContextualOS, Reject) {
        Context context{2};
        std::stringstream ss;
        ContextualOS cSS{ss, context};

        RejectsCOSObject the_obj{1337};
        cSS << the_obj;
        EXPECT_EQ(ss.str(), "1337");
    }

    TEST(Scenarios_ContextualOS, Reject_BuiltIn) {
        Context context{2};
        std::stringstream ss;
        ContextualOS cSS{ss, context};
        cSS << "Hello world";

        EXPECT_EQ(ss.str(), "Hello world");
    }

    TEST(Scenarios_ContextualOS, Accept) {
        Context context{2};
        std::stringstream ss;
        ContextualOS cSS{ss, context};

        AcceptsCOSObject the_obj{1337};
        cSS << the_obj;
        EXPECT_EQ(ss.str(), "2:1337");
    }

    TEST(Scenarios_ContextualOS, Functor) {
        Algebraic::AlgebraicMatrixSystem ams{std::make_unique<Algebraic::AlgebraicContext>(2)};
        const SymbolTable& symbol = ams.Symbols();
        const Context& context = ams.Context();

        OperatorSequence op_seq{{0,1}, context};

        auto output = make_contextualized_string(context, symbol, [&](ContextualOS& os) {
            os << op_seq;
        });

        EXPECT_EQ(output, "X1;X2");

    }

    TEST(Scenarios_ContextualOS, FunctorFromSFC) {
        Algebraic::AlgebraicMatrixSystem ams{std::make_unique<Algebraic::AlgebraicContext>(2)};
        const SymbolTable& symbol = ams.Symbols();
        const Context& context = ams.Context();

        OperatorSequence op_seq{{0,1}, context};
        StringFormatContext sfc{context, symbol};
        sfc.format_info.show_braces = true;

        auto output = make_contextualized_string(sfc, [&](ContextualOS& os) {
            os << op_seq;
        });

        EXPECT_EQ(output, "<X1;X2>");

    }

}