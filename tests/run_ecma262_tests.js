// Copyright (c) 2014 Cesanta Software
// All rights reserved

var numTotalCases = 0, numFailedTests = 0, numExceptions = 0, numPassed = 0;


function f(n) {
  n = 200;
  print('before: ', n);
  for (var k = 0; k >= 0; k--) {
    //if (n >= Math.pow(16, i)) {
    //var t = Math.floor(n / Math.pow(16, i));
    print(n, ' ', k);
      n += 10;
      print(n, ' ', t);
    //return n;
    //}
  }
  print('after: ', n);
  return n;
}

for (var i = 0; i < 2; i++) {
  f(i);
}



exit(0);

// Run ECMAScript conformance tests, taken from
// http://test262.ecmascript.org/json/ch09.json
var testFiles = ['ch06.json', 'ch07.json', 'ch09.json'];


var $ERROR = function(msg) {
  print(msg);
  numFailedTests++;
};

for (var i = 0; i < testFiles.length; i++) {
  var cfg = load('ecma262/' + testFiles[i]);
  for (var j = 0; j < cfg.testsCollection.tests.length; j++) {
    var test = cfg.testsCollection.tests[j];
    var code = base64_decode(test.code);
    //print('----> ', cfg.testsCollection.name, ' :: ', test.path);
    numTotalCases++;
    try {
      var currFailed = numFailedTests;
      eval(code);
      if (currFailed == numFailedTests) numPassed++;
    } catch (err) {
      print(code, '============= ', test.path, '\n\n\n');
      numExceptions++;
    }
  }
}

print('Total cases: ', numTotalCases,
      ', passed cases: ', numPassed,
      ', failed checks: ', numFailedTests,
      ', exceptions: ', numExceptions);