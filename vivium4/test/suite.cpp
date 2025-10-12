#include "suite.h"

namespace Vivium {
TestResult testFailed(std::string reason) {
  TestResult result;
  result.text = reason;
  result.didPass = false;

  return result;
}

TestResult testPassed(std::string message) {
  TestResult result;
  result.text = message;
  result.didPass = true;

  return result;
}

std::string defaultFormatSuite(TestFormatter const& format,
                               TestSuiteContext const& ctx) {
  return std::format("Suite {}; {} tests\n", ctx.suiteName, ctx.testCount);
}

std::string defaultFormatHeader(TestFormatter const& format,
                                TestHeaderContext const& ctx) {
  return std::format("\t{}; running {} tests\n", ctx.headerText, ctx.testCount);
}

std::string defaultFormatTest(TestFormatter const& format,
                              TestContext const& ctx) {
  return std::format("\t\t{}: {}\n", ctx.testName,
                     format.formatResult(format, ctx.result));
}

std::string defaultFormatResult(TestFormatter const& format,
                                TestResult const& result) {
  std::string_view passText = result.didPass ? "PASS" : "FAIL";
  std::string_view continuation = result.text.size() ? ": " : "";

  return std::string(passText) + std::string(continuation) + result.text;
}

std::string defaultFormatFinish(TestFormatter const& format,
                                TestFinishContext const& context) {
  int failedTests = context.failedTests.size();
  int passedTests = context.totalTestsRun - failedTests;
  float percentage =
      passedTests / static_cast<float>(context.totalTestsRun) * 100.0f;

  std::ostringstream failedTestString;

  // TODO: the header?
  // TODO: maybe remove entirely
  for (TestContext const& test : context.failedTests) {
    failedTestString << std::format("{}\n", format.formatTest(format, test));
  }

  return std::format("Passed [{}/{}] {:.2f}%, printing failed tests: {}",
                     passedTests, context.totalTestsRun, percentage,
                     failedTestString.str());
}

TestFormatter defaultFormatter() {
  TestFormatter formatter;
  formatter.formatResult = defaultFormatResult;
  formatter.formatTest = defaultFormatTest;
  formatter.formatFinish = defaultFormatFinish;
  formatter.formatHeader = defaultFormatHeader;
  formatter.formatSuite = defaultFormatSuite;

  return formatter;
}

TestSuite createSuite(std::string_view suiteName, TestFormatter formatter,
                      TestOutstream outstream) {
  TestSuite suite;
  suite.name = suiteName;
  suite.formatter = formatter;
  suite.outstream = outstream;

  return suite;
}

TestHeader pushHeader(TestSuite& suite, std::string_view headerName) {
  TestHeader header;
  header.name = headerName;

  suite.headers.push_back(header);

  return header;
}

TestResult pushResult(TestSuite& suite, std::string_view name,
                      TestResult const& result) {
  if (suite.headers.empty()) {
    // TODO: headers shouldn't be necessary
    VIVIUM_LOG(LogSeverity::FATAL, "Added result without header");
  }

  Test test;
  test.name = name;
  test.result = result;

  suite.headers.back().tests.push_back(test);

  return result;
}

TestFinish finishSuite(TestSuite& suite) {
  TestFinish finish;

  finish.testResults = {};
  finish.totalTests = 0;

  for (TestHeader const& header : suite.headers) {
    finish.totalTests += header.tests.size();

    for (Test const& test : header.tests) {
      finish.testResults.push_back(test);
    }
  }

  return finish;
}

void printSuite(TestSuite& suite) {
  TestSuiteContext suiteContext;
  suiteContext.suiteName = suite.name;
  suiteContext.testCount = 0;
  suiteContext.headerCount = suite.headers.size();

  TestFormatter const& format = suite.formatter;

  for (TestHeader const& header : suite.headers) {
    suiteContext.testCount = header.tests.size();
  }

  std::cout << format.formatSuite(format, suiteContext);

  for (TestHeader const& header : suite.headers) {
    TestHeaderContext headerContext;
    headerContext.testCount = header.tests.size();
    headerContext.suiteName = suite.name;
    headerContext.headerText = header.name;

    std::cout << format.formatHeader(format, headerContext);

    for (Test const& test : header.tests) {
      TestContext testContext;
      testContext.header = headerContext;
      testContext.result = test.result;
      testContext.testName = test.name;

      std::cout << format.formatTest(format, testContext);
    }
  }

  std::cout << "Test Complete" << std::endl;
}
}  // namespace Vivium
