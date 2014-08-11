load('tests/unit_test.js');


// Run ECMAScript conformance tests, taken from
// http://test262.ecmascript.org/json/ch09.json
var testFiles = ['ch09.json'];

var $ERROR = print;

for (var i = 0; i < testFiles.length; i++) {
  var cfg = load('tests/' + testFiles[i]);
  print('Running ECMAScript test: "', cfg.testsCollection.name, '"');

  for (var j = 0; j < cfg.testsCollection.tests.length; j++) {
    var test = cfg.testsCollection.tests[j];
    var code = base64_decode(test.code);
    //print('  ', test.description);
    //print(code);
    //eval(code);
  }
}
