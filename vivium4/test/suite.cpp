#include "suite.h"

namespace Vivium {
void consoleOutstream(TestSuite const& suite, std::string const& text) {
  std::cout << text;
}

TestResult testFailed(std::string reason) {
  TestResult result;
  result.text = reason;
  result.type = TestResultType::FAIL;

  return result;
}

TestResult testPassed(std::string message) {
  TestResult result;
  result.text = message;
  result.type = TestResultType::OK;

  return result;
}

TestResult testFatalFailed(std::string message) {
  TestResult result;
  result.text = message;
  result.type = TestResultType::FATAL;

  return result;
}

std::string defaultFormatSuite(TestFormatter const& format,
                               TestSuiteContext const& ctx) {
  return std::format("Starting suite {}\n", ctx.suiteName);
}

std::string defaultFormatHeader(TestFormatter const& format,
                                TestHeaderContext const& ctx) {
  std::string whitespace = std::string(ctx.indentLevel, '\t');

  return std::format("{}{}\n", whitespace, ctx.headerText);
}

std::string defaultFormatTest(TestFormatter const& format,
                              TestContext const& ctx) {
  std::string whitespace = std::string(ctx.header.indentLevel + 1, '\t');

  return std::format("{}{}: {}\n", whitespace, ctx.testName,
                     format.formatResult(format, ctx.result));
}

std::string defaultFormatResult(TestFormatter const& format,
                                TestResult const& result) {
  std::string_view passText =
      (result.type == TestResultType::OK) ? "PASS" : "FAIL";
  std::string_view continuation = result.text.size() ? ": " : "";

  return std::string(passText) + std::string(continuation) + result.text;
}

std::string defaultFormatFinish(TestFormatter const& format,
                                TestFinishContext const& context) {
  int failedTests = context.failedTests.size();
  int passedTests = context.totalTestsRun - failedTests;
  float percentage =
      passedTests / static_cast<float>(context.totalTestsRun) * 100.0f;

  return std::format("Passed [{}/{}] {:.2f}%\n", passedTests,
                     context.totalTestsRun, percentage);
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

void _dfsHeader(TestHeader const& header,
                std::function<void(TestHeader const&)> const& headerFunc,
                std::function<void(Test const&)> const& testFunc,
                bool runHead) {
  if (runHead) {
    headerFunc(header);

    for (Test const& test : header.tests) {
      testFunc(test);
    }
  }

  for (TestHeader const& child : header.headers) {
    headerFunc(child);

    for (Test const& test : child.tests) {
      testFunc(test);
    }

    _dfsHeader(child, headerFunc, testFunc, false);
  }
}

TestSuite createSuite(std::string_view suiteName, TestFormatter formatter,
                      TestOutstream outstream) {
  TestSuite suite;
  suite.name = suiteName;
  suite.formatter = formatter;
  suite.outstream = outstream;
  suite.headerDepth = 0;
  suite.encounteredFatal = false;

  TestSuiteContext context;
  context.suiteName = suite.name;

  suite.outstream(suite, suite.formatter.formatSuite(suite.formatter, context));

  return suite;
}

TestHeader pushHeader(TestSuite& suite, std::string_view headerName) {
  TestHeader header;
  header.name = headerName;
  header.depth = ++suite.headerDepth;

  suite.headers.push_back(header);

  TestHeaderContext context;
  context.indentLevel = header.depth;
  context.headerText = header.name;
  context.suiteName = suite.name;

  suite.outstream(suite,
                  suite.formatter.formatHeader(suite.formatter, context));

  return header;
}

void endHeader(TestSuite& suite) { suite.headerDepth--; }

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

  TestHeader const& header = suite.headers.back();

  TestContext context;
  TestHeaderContext headerContext;
  headerContext.indentLevel = header.depth;
  headerContext.headerText = header.name;
  headerContext.suiteName = suite.name;

  context.header = headerContext;
  context.result = result;
  context.testName = name;

  suite.outstream(suite, suite.formatter.formatTest(suite.formatter, context));

  if (test.result.type == TestResultType::FATAL) {
    suite.encounteredFatal = true;
  }

  return result;
}

TestFinish finishSuite(TestSuite& suite) {
  TestFinish finish;
  TestFinishContext context;

  finish.testResults = {};
  finish.totalTests = 0;
  context.failedTests = {};

  for (TestHeader const& header : suite.headers) {
    _dfsHeader(
        header,
        [&finish](TestHeader const& child) {
          finish.totalTests += child.tests.size();
        },
        [&finish, &context, &suite, &header](Test const& child) {
          finish.testResults.push_back(child);

          if (child.result.type != TestResultType::OK) {
            TestHeaderContext headerContext;
            headerContext.suiteName = suite.name;
            headerContext.headerText = header.name;
            headerContext.indentLevel = header.depth;

            TestContext testContext;
            testContext.result = child.result;
            testContext.header = headerContext;
            testContext.testName = child.name;

            context.failedTests.push_back(testContext);
          }
        },
        true);
  }

  context.totalTestsRun = finish.totalTests;

  suite.outstream(suite,
                  suite.formatter.formatFinish(suite.formatter, context));

  return finish;
}

}  // namespace Vivium
