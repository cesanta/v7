// Copyright (c) 2004-2013 Sergey Lyubka <valenok@gmail.com>
// Copyright (c) 2013-2014 Cesanta Software Limited
// All rights reserved
//
// To run this test, start a terminal and execute "make js"

var numFailedTests = 0, numPassedTests = 0;
function test(condition, msg) {
  if (condition) {
    numPassedTests++;
  } else {
    print(msg, ' test failed');
    numFailedTests++;
  }
}


var func1 = function() {
  var param = 5;
  param++;
  return function(x, y) { return x * y * param; }
}
var closure1 = func1(), closure2 = func1();
test(closure1(2, 3) == 36, 'closure1');
test(closure2(2, 3) == 36, 'closure2');


var v1 = 123, v2 = 234;
function f(v1) { return v1 ? v1 : v2; }
test(f(1) == 1, 'func params 1');
test(f() == 234, 'func params 2');


var a = 5, b = a;
test(b == 5, 'assign 1');
a = 7;
test(b == 5, 'assign 2');
a = { foo: 2 }; b = a;
test(b.foo == 2, 'assign 3');
a.foo = 7;
test(b.foo == 7, 'assign 4');


(function() {
  var a = { b: 'aa bb cc dd' };
  var x = a.b.substr().split('\r\n');
})();

var a = { b: function() { return this.c; }, c: 777 };
test(a.b() == 777, 'this 1');
test(this.a.c == 777, 'this 2');

var factorial = function(x) { return x <= 1 ? 1 : x * factorial(x - 1); };
test(factorial(5) == 120, 'factorial 1');
test(factorial(-2) == 1, 'factorial 2');
test(factorial(1) == 1, 'factorial 3');


var module = (function(arg) {
  var privateProperty = 17;
  var privateMethod = function(x, y) { return x * y * private_var * arg; };
  return {
    publicProperty: 'I am public!',
    publicMethod: function(x) { return x * arg; },
    privilegedMethod: function(x, y) { return privateMethod(x, y); }
  };
})(7);
test(module.publicProperty === 'I am public!');
//test(module.publicMethod(3) === 51);
//test(module.privilegedMethod(3, 4) === 1020);


test('a' + 'b' == 'ab', 'string concatenation');
test('He who knows, does not speak'.substr(3, 3) == 'who', 'substr 1');
test('He who speaks, does not know'.substr(24) == 'know', 'substr 2');
test('abc'.substr() === 'abc', 'substr 3');
test('A warrior never worries about his fear'.indexOf('his') == 30, 'indexOf');
test('xyz'.substr().indexOf('y') == 1, 'indexOf2');
test((123.33245).toFixed(0) === '123', 'toFixed');
test(($a$_$ = 17) == 17, '$ in identifier');
test(Math.PI - 3.1415926 < 0.0001, 'Math 1');
test(1.2345 / 0.0 == Infinity, 'Math 2');
test(undefined == undefined, 'undef');
test(undefined == null, 'undef2');
test((function(x,y){ return x * y;})(3, 4) == 12, 'anon function');

print('Passed tests: ', numPassedTests, ', failed tests: ', numFailedTests);
