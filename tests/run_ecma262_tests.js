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

function MyObject( val ) {
  this.value = val;
  this.valueOf = function (){ return this.value; }
}

var x = new MyObject(1);
var y = Object(x);

// CHECK#1
if (y.valueOf !== x.valueOf){
  $ERROR('#1: Object(obj).valueOf() === obj.valueOf(). Actual: ' + 22);
}
/*

// CHECK#2
if (typeof y !== typeof x){
  $ERROR('#2: typeof Object(obj) === typeof obj. Actual: ' + (typeof Object(obj)));
}

// CHECK#3
if (y.constructor.prototype !== x.constructor.prototype){
  $ERROR('#3: Object(obj).constructor.prototype === obj.constructor.prototype. Actual: ' + (Object(obj).constructor.prototype));
}


// CHECK#4
if (y !== x){
  $ERROR('#4: Object(obj) === obj');
}
 */

exit(0);


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
      print(code, '============= ', test.path, ' (', ok, ')\n\n\n\n');
      numExceptions++;
    }
  }
}

print('Total cases: ', numTotalTests,
      ', failed checks: ', numFailedTests,
      ', exceptions: ', numExceptions);