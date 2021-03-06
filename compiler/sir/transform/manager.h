#pragma once

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "pass.h"
#include "sir/analyze/analysis.h"
#include "sir/module.h"

namespace seq {
namespace ir {
namespace transform {

/// Utility class to run a series of passes.
class PassManager {
private:
  /// Container for pass metadata.
  struct PassMetadata {
    /// pointer to the pass instance
    std::unique_ptr<Pass> pass;
    /// vector of required analyses
    std::vector<std::string> reqs;
    /// vector of invalidated analyses
    std::vector<std::string> invalidates;

    PassMetadata() = default;
    PassMetadata(std::unique_ptr<Pass> pass, std::vector<std::string> reqs,
                 std::vector<std::string> invalidates)
        : pass(std::move(pass)), reqs(std::move(reqs)),
          invalidates(std::move(invalidates)) {}
    PassMetadata(PassMetadata &&) = default;

    PassMetadata &operator=(PassMetadata &&) = default;
  };

  /// Container for analysis metadata.
  struct AnalysisMetadata {
    /// pointer to the analysis instance
    std::unique_ptr<analyze::Analysis> analysis;
    /// vector of required analyses
    std::vector<std::string> reqs;
    /// vector of invalidated analyses
    std::vector<std::string> invalidates;

    AnalysisMetadata() = default;
    AnalysisMetadata(std::unique_ptr<analyze::Analysis> analysis,
                     std::vector<std::string> reqs)
        : analysis(std::move(analysis)), reqs(std::move(reqs)) {}
    AnalysisMetadata(AnalysisMetadata &&) = default;

    AnalysisMetadata &operator=(AnalysisMetadata &&) = default;
  };

  /// map of keys to passes
  std::unordered_map<std::string, PassMetadata> passes;
  /// map of keys to analyses
  std::unordered_map<std::string, AnalysisMetadata> analyses;
  /// reverse dependency map
  std::unordered_map<std::string, std::vector<std::string>> deps;

  /// execution order of passes
  std::vector<std::string> executionOrder;
  /// map of valid analysis results
  std::unordered_map<std::string, std::unique_ptr<analyze::Result>> results;

public:
  /// Registers a pass and appends it to the execution order.
  /// @param key the pass's key
  /// @param pass the pass
  /// @param reqs keys of analyses that must be run before the current one
  /// @param invalidates keys of analyses that are invalidated by the current one
  void registerPass(const std::string &key, std::unique_ptr<Pass> pass,
                    std::vector<std::string> reqs = {},
                    std::vector<std::string> invalidates = {});

  /// Registers an analysis.
  /// @param key the analysis's key
  /// @param analysis the analysis
  /// @param reqs keys of analyses that must be run before the current one
  void registerAnalysis(const std::string &key,
                        std::unique_ptr<analyze::Analysis> analysis,
                        std::vector<std::string> reqs = {});

  /// Run all passes.
  /// @param module the module
  void run(Module *module);

  /// Gets the result of a given analysis.
  /// @param key the analysis key
  /// @return the result
  analyze::Result *getAnalysisResult(const std::string &key) {
    auto it = results.find(key);
    return it != results.end() ? it->second.get() : nullptr;
  }

private:
  void runPass(Module *module, const std::string &name);
  void runAnalysis(Module *module, const std::string &name);
  void invalidate(const std::string &key);
};

} // namespace transform
} // namespace ir
} // namespace seq
