#include "test.h"

#include <sstream>

#include "util/fmt/format.h"

using namespace seq::ir;

TEST_F(SIRCoreTest, ConstTypeQueryAndReplace) {
  auto *node = module->Nr<IntConst>(1, module->getIntType());
  ASSERT_EQ(module->getIntType(), node->getType());

  auto usedTypes = node->getUsedTypes();
  ASSERT_EQ(1, usedTypes.size());
  ASSERT_EQ(module->getIntType(), usedTypes[0]);
  ASSERT_EQ(1, node->replaceUsedType(module->getIntType(), module->getIntType()));
}

TEST_F(SIRCoreTest, ConstValueMatches) {
  auto VALUE = 1;

  auto *node = module->Nr<IntConst>(VALUE, module->getIntType());
  ASSERT_EQ(VALUE, node->getVal());

  std::stringstream s;
  s << *node;
  ASSERT_EQ(std::to_string(VALUE), s.str());
}

TEST_F(SIRCoreTest, ConstCloning) {
  auto VALUE = 1;
  auto *node = module->Nr<IntConst>(VALUE, module->getIntType());
  auto *clone = cast<IntConst>(cv->clone(node));

  ASSERT_TRUE(clone);
  ASSERT_EQ(VALUE, clone->getVal());
  ASSERT_EQ(module->getIntType(), clone->getType());
}

TEST_F(SIRCoreTest, StringConstFormatting) {
  auto VALUE = "hi";

  auto *node = module->Nr<StringConst>(VALUE, module->getStringType());

  std::stringstream s;
  s << *node;
  ASSERT_EQ(fmt::format(FMT_STRING("\"{}\""), VALUE), s.str());
}
