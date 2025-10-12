#pragma once

#include <format>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "../error/log.h"

namespace Vivium {
struct TestResult {
  std::string text;
  bool didPass;
};

TestResult testFailed(std::string reason);
TestResult testPassed(std::string message);

struct TestSuiteContext {
  std::string suiteName;
  int headerCount;
  int testCount;
};

struct TestHeaderContext {
  std::string suiteName;
  std::string headerText;
  int testCount;
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
using TestOutstream = void (*)(TestSuite const&, std::string const&);

// TODO: implementation
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
  std::vector<Test> tests;
};

struct TestSuite {
  std::string name;
  std::vector<TestHeader> headers;
  TestFormatter formatter;
  TestOutstream outstream;
};

struct TestFinish {
  std::vector<Test> testResults;
  int totalTests;
};

TestSuite createSuite(std::string_view suiteName, TestFormatter formatter,
                      TestOutstream stream);
// TODO: headers could also operate like a stack?
// would require pops..
TestHeader pushHeader(TestSuite& suite, std::string_view headerName);
TestResult pushResult(TestSuite& suite, std::string_view name,
                      TestResult const& result);
TestFinish finishSuite(TestSuite& suite);

// TODO: not great, we want live formatting and printing
// TODO: also we are manually creating all the contexts
void printSuite(TestSuite& suite);
}  // namespace Vivium
