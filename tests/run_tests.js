load('tests/unit_test.js');

// Run ECMAScript conformance tests, taken from
// http://test262.ecmascript.org/json/ch09.json
var testFiles = ['ch06.json', 'ch09.json'];
var numTotalTests = 0;
var numFailedTests = 0;

var $ERROR = function(msg) {
  //print(msg);
  numFailedTests++;
};

for (var i = 0; i < testFiles.length; i++) {
  var cfg = load('tests/' + testFiles[i]);
  print('  ==> Running "', cfg.testsCollection.name, '"');

  for (var j = 0; j < cfg.testsCollection.tests.length; j++) {
    var test = cfg.testsCollection.tests[j];
    var code = base64_decode(test.code);
    numTotalTests++;
    try {
      eval(code);
    } catch (err) {
      print('Error running ', test.path, ': ', err, '\n', code, '\n========');
    }
  }
}

print('Total tests: ', numTotalTests, ', failed: ', numFailedTests);