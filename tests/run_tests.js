load('tests/unit_test.js');

// Run ECMAScript conformance tests, taken from
// http://test262.ecmascript.org/json/ch09.json
var testFiles = ['ch09.json'];

for (var i = 0; i < testFiles.length; i++) {
  var cfg = load('tests/' + testFiles[i]);
  printf('Testing ', cfg.testsCollection.name, ' ...');
}
