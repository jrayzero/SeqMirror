#include "test.h"

#include "sir/transform/manager.h"
#include "sir/transform/pass.h"

using namespace seq::ir;

class DummyResult : public analyze::Result {};

class DummyAnalysis : public analyze::Analysis {
private:
  int &counter;

public:
  static int runCounter;

  explicit DummyAnalysis(int &counter) : counter(counter) {}

  std::unique_ptr<analyze::Result> run(const Module *) override {
    runCounter = counter++;
    return std::make_unique<DummyResult>();
  }
};

int DummyAnalysis::runCounter = 0;

class DummyPass : public transform::Pass {
private:
  int &counter;
  std::string required;

public:
  static int runCounter;

  explicit DummyPass(int &counter, std::string required)
      : counter(counter), required(std::move(required)) {}

  void run(Module *) override {
    runCounter = counter++;
    ASSERT_TRUE(getAnalysisResult<DummyResult>(required));
  }
};

int DummyPass::runCounter = 0;

TEST_F(SIRCoreTest, PassManagerNoInvalidations) {
  auto ANALYSIS_KEY = "**test_analysis**";
  auto PASS_KEY = "**test_pass**";
  int counter = 0;

  auto manager = std::make_unique<transform::PassManager>();
  manager->registerAnalysis(ANALYSIS_KEY, std::make_unique<DummyAnalysis>(counter));
  manager->registerPass(PASS_KEY, std::make_unique<DummyPass>(counter, ANALYSIS_KEY),
                        {ANALYSIS_KEY});
  manager->run(module.get());

  ASSERT_EQ(0, DummyAnalysis::runCounter);
  ASSERT_EQ(1, DummyPass::runCounter);
}

TEST_F(SIRCoreTest, PassManagerInvalidations) {
  auto ANALYSIS_KEY = "**test_analysis**";
  auto PASS_KEY = "**test_pass**";
  auto PASS_KEY_2 = "**test_pass2**";

  int counter = 0;

  auto manager = std::make_unique<transform::PassManager>();
  manager->registerAnalysis(ANALYSIS_KEY, std::make_unique<DummyAnalysis>(counter));
  manager->registerPass(PASS_KEY, std::make_unique<DummyPass>(counter, ANALYSIS_KEY),
                        {ANALYSIS_KEY}, {ANALYSIS_KEY});
  manager->registerPass(PASS_KEY_2, std::make_unique<DummyPass>(counter, ANALYSIS_KEY),
                        {ANALYSIS_KEY});

  manager->run(module.get());

  ASSERT_EQ(2, DummyAnalysis::runCounter);
  ASSERT_EQ(3, DummyPass::runCounter);
}
