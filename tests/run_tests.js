var testFiles = ['ch09.json'];

load('unit_test.js');

for (var i = 0; i < testFiles.length; i++) {
  //var path = 'tests/' + testFiles[i];
  var cfg = load('tests/' + testFiles[i]);
  printf('Testing ', cfg.testsCollection.name, ' ...');
}

//var o = load('tests/ch09.json');

//print(o.testsCollection.tests.length);
//print(o.testsCollection.numTests);