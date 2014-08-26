// Copyright (c) 2004-2013 Sergey Lyubka <valenok@gmail.com>
// Copyright (c) 2013-2014 Cesanta Software Limited
// All rights reserved
//
// To run this test, start a terminal and run "make js"


var numFailedTests = 0, numPassedTests = 0;
function test(condition, msg) {
  if (condition) {
    numPassedTests++;
  } else {
    print(msg, ' test failed');
    numFailedTests++;
  }
}


var func1 = function(x) {
  var param = 4;
  param++;
  return function(y) { return x * y * param; }
}

var closure1 = func1(2), closure2 = func1(3);
test(closure1(7) == 70, 'closure1');
test(closure2(7) == 105, 'closure2');


var func2 = function(x) { x = 10; test(x == 10, 'local scope 1'); }
func2(3);

var v1 = 123, v2 = 234;
function f(v1) { return v1 ? v1 : v2; }
test(f(1) == 1, 'func params 1');
test(f() == 234, 'func params 2');


var a = 5, b = a;
test(b == 5, 'assign 1');
a = 7;
test(b == 5, 'assign 2');
a -= 2;
test(a == 5, 'incr3');
a = { foo: 2 }; b = a;
test(b.foo == 2, 'assign 3');
a.foo = 7;
test(b.foo == 7, 'assign 4');
a.foo++;
test(b.foo == 8, 'incr1');
a.foo -= 7;
test(b.foo == 1, 'incr2');


a = 1, b = 1;
for (var i = 1; i <= 5; i++) { a *= i; }
for (i = 1; i <= 6; i++) b *= i;
test(a == 120, 'for1');
test(b == 720, 'for2');

a = { foo: 1, bar: 2, baz: 3 }, b = [];
for (var key in a) b.push(key);
b.sort();
test(b.length == 3 && b[0] == 'bar' && b[1] == 'baz' && b[2] == 'foo', 'for3');

(function() {
  var a = { b: 'aa bb cc dd' };
  var x = a.b.substr().split('\r\n');
})();

var a = { b: function() { return this.c; }, c: 777 };
test(a.b() == 777, 'this 1');
test(this.a.c == 777, 'this 2');

var a = 11, hoisting1 = false;
var b = function() { return a; var a = 'hi'; }
try { b(); ff(); } catch (e) { hoisting1 = true; a = 12; }
// test(hoisting1, 'hoisting1'); // Rhino does not support strict


var factorial = function(x) { return x <= 1 ? 1 : x * factorial(x - 1); };
test(factorial(5) == 120, 'factorial 1');
test(factorial(-2) == 1, 'factorial 2');
test(factorial(1) == 1, 'factorial 3');


var ctor = function(x) { this.prop1 = x; return 0; };
var o = new ctor('blah');
test(o.prop1 === 'blah', 'ctor1');


var module = (function(arg) {
  var privateProperty = 17;
  var privateMethod = function(x, y) { return x * y * privateProperty * arg; };
  return {
    publicProperty: 'I am public!',
    publicMethod: function(x) { return x * arg; },
    privilegedMethod: function(x, y) { return privateMethod(x, y); }
  };
})(7);
test(module.publicProperty === 'I am public!', 'public prop');
test(module.publicMethod(3) == 3 * 7, 'public method');
test(module.privilegedMethod(3, 4) === 3 * 4 * 17 * 7, 'privilegedMethod');

var httpRequest = 'GET / HTTP/1.0\r\nFoo: bar\r\nBaz: xx yy\r\n\r\n';
var lines = httpRequest.split(/\s*\n\s*/);
test(lines.length == 4, 'split4');
test(lines[3].length === 0, 'split5');
test(lines[2] === 'Baz: xx yy' , 'split6');

var a = 0, b, c = '';
var foo = function() { return a++ < 10 ? 'x' : false };
while (b = foo()) { c += b; }
test(c === 'xxxxxxxxxx', 'while1')

test('a' + 'b' == 'ab', 'string concatenation');
test('He who knows, does not speak'.substr(3, 3) == 'who', 'substr 1');
test('He who speaks, does not know'.substr(24) == 'know', 'substr 2');
test('abc'.substr() === 'abc', 'substr 3');
test('A warrior never worries about his fear'.indexOf('his') == 30, 'indexOf');
test('xyz'.substr().indexOf('y') == 1, 'indexOf2');
test('abc' + 123 == 'abc123', 'string and int concatenation');
test('abc' + 123.3 == 'abc123.3', 'string and float concatenation');
test('abc' + new Number(123.3) == 'abc123.3', 'string and Number concatenation');
test((123.33245).toFixed(0) === '123', 'toFixed');
test(($a$_$ = 17) == 17, '$ in identifier');
test(Math.PI - 3.1415926 < 0.0001, 'Math 1');
test(1.2345 / 0.0 == Infinity, 'Math 2');
test(undefined == undefined, 'undef');
test(undefined == null, 'undef2');
test(Number(3.45) == 3.45, 'new1');
test((new Number(3.45)) == 3.45, 'new2');
test((new String()) == '', 'new3');
test((function(x,y){ return x * y;})(3, 4) == 12, 'anon function');
test(typeof 123 === 'number', 'typeof1');
test(typeof {} === 'object', 'typeof2');
test(typeof function(){} === 'function', 'typeof3');
test(typeof 'x' === 'string', 'typeof4');
test(typeof [] === 'object', 'typeof5');
test(typeof null === 'object', 'typeof6');
test(typeof (new String()) === 'object', 'new4');
test(typeof (new Number(3.45)) == 'object', 'new5');
test('aa  \tbb cc'.split(/\s+/).length == 3, 'split1');
test('aa  \tbb cc'.split(/\s+/)[1] == 'bb', 'split2');
test('aa  \tbb cc'.split(/\s+/)[2] == 'cc', 'split3');
test([] instanceof Array, 'instanceof1');
test(new String() instanceof String, 'instanceof2');
test(function(){} instanceof Function, 'instanceof3');
test(function(){} instanceof Object, 'instanceof4');
test(new (function(){})() instanceof Object, 'instanceof5');
test(new String() instanceof Object, 'instanceof6');
test(new Number(1) instanceof Number, 'instanceof7');


if (!constructor) {
  // Rhino doesn't have that
  test(Crypto.md5_hex('') == 'd41d8cd98f00b204e9800998ecf8427e', 'md5_hex1');
  test(Crypto.md5_hex('x') == '9dd4e461268c8034f5c8564e155c67a6', 'md5_hex2');
  test(Crypto.sha1_hex('foo') == '0beec7b5ea3f0fdbc95d0dd47f3c5bc275da8a33',
       'sha1_hex1');
}


print('Passed tests: ', numPassedTests, ', failed tests: ', numFailedTests);