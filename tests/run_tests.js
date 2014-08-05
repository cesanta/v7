load('tests/unit_test.js');

// Run ECMAScript conformance tests, taken from
// http://test262.ecmascript.org/json/ch09.json
var testFiles = ['ch09.json'];

for (var i = 0; i < testFiles.length; i++) {

  var cfg = load('tests/' + testFiles[i]);
  print('Running ECMAScript test: "', cfg.testsCollection.name, '"');

  for (var j = 0; j < cfg.testsCollection.tests.length; j++) {
    var test = cfg.testsCollection.tests[j];
    // TODO(lsm): base64-decode test.code and run it.
    //print(test.code);
  }
}
