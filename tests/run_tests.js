// Copyright (c) 2014 Cesanta Software
// All rights reserved

String(null);

// Run ECMAScript conformance tests, taken from
// http://test262.ecmascript.org/json/ch09.json
var testFiles = ['ch06.json', 'ch09.json'];
var numTotalTests = 0;
var numFailedTests = 0;
var numExceptions = 0;

var $ERROR = function(msg) {
  print(msg);
  numFailedTests++;
};

for (var i = 0; i < testFiles.length; i++) {
  var cfg = load('tests/' + testFiles[i]);
  print('  ==> Running "', cfg.testsCollection.name, '"');

  for (var j = 0; j < cfg.testsCollection.tests.length; j++) {
    var test = cfg.testsCollection.tests[j];
    var code = base64_decode(test.code);
    var ok = 'failed';
    numTotalTests++;
    try {
      var currFailed = numFailedTests;
      eval(code);
      if (currFailed == numFailedTests) ok = 'passed';
    } catch (err) {
      numExceptions++;
    }
    //print(code, '============= ', test.path, ' (', ok, ')\n\n\n\n');
  }
}

print('Total cases: ', numTotalTests,
      ', failed checks: ', numFailedTests,
      ', exceptions: ', numExceptions);