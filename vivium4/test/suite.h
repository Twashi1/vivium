#pragma once

#include <format>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "../error/log.h"

namespace Vivium {
enum class TestResultType { OK, FAIL, FATAL };

struct TestResult {
  std::string text;
  TestResultType type;
};

TestResult testFailed(std::string reason);
TestResult testFatalFailed(std::string message);
TestResult testPassed(std::string message);

struct TestSuiteContext {
  std::string suiteName;
};

struct TestHeaderContext {
  std::string suiteName;
  std::string headerText;
  int indentLevel;
};

struct TestContext {
  std::string testName;
  TestResult result;
  TestHeaderContext header;
};

struct TestFinishContext {
  std::vector<TestContext> failedTests;
  int totalTestsRun;
};

struct TestFormatter;

using TestFormatSuite = std::string (*)(TestFormatter const&,
                                        TestSuiteContext const&);
using TestFormatHeader = std::string (*)(TestFormatter const&,
                                         TestHeaderContext const&);
using TestFormatTest = std::string (*)(TestFormatter const&,
                                       TestContext const&);
using TestFormatResult = std::string (*)(TestFormatter const&,
                                         TestResult const&);
using TestFormatFinish = std::string (*)(TestFormatter const&,
                                         TestFinishContext const&);

struct TestSuite;

using TestOutstream = void (*)(TestSuite const&, std::string const&);

void consoleOutstream(TestSuite const& suite, std::string const& text);

std::string defaultFormatSuite(TestFormatter const& format,
                               TestSuiteContext const& ctx);
std::string defaultFormatHeader(TestFormatter const& format,
                                TestHeaderContext const& ctx);
std::string defaultFormatTest(TestFormatter const& format,
                              TestContext const& ctx);
std::string defaultFormatResult(TestFormatter const& format,
                                TestResult const& result);
std::string defaultFormatFinish(TestFormatter const& format,
                                TestFinishContext const& context);

struct TestFormatter {
  TestFormatSuite formatSuite;
  TestFormatHeader formatHeader;
  TestFormatTest formatTest;
  TestFormatResult formatResult;
  TestFormatFinish formatFinish;
};

TestFormatter defaultFormatter();

struct Test {
  TestResult result;
  std::string name;
};

struct TestHeader {
  std::string name;
  std::vector<TestHeader> headers;
  std::vector<Test> tests;
  int depth;
};

struct TestSuite {
  std::string name;
  std::vector<TestHeader> headers;
  TestFormatter formatter;
  TestOutstream outstream;
  int headerDepth;
  bool encounteredFatal;
};

struct TestFinish {
  std::vector<Test> testResults;
  int totalTests;
};

template <typename T>
concept ValidTestFunctor = requires(T f) {
  { f() } -> std::same_as<TestResult>;
};

void _dfsHeader(TestHeader const& header,
                std::function<void(TestHeader const&)> const& headerFunc,
                std::function<void(Test const&)> const& testFunc, bool runHead);

TestSuite createSuite(std::string_view suiteName, TestFormatter formatter,
                      TestOutstream stream);
TestHeader pushHeader(TestSuite& suite, std::string_view headerName);
void endHeader(TestSuite& suite);

TestResult pushResult(TestSuite& suite, std::string_view name,
                      TestResult const& result);
template <ValidTestFunctor Functor>
void pushTest(TestSuite& suite, std::string_view name, Functor testCode);
TestFinish finishSuite(TestSuite& suite);
}  // namespace Vivium
