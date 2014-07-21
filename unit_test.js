// Copyright (c) 2004-2013 Sergey Lyubka <valenok@gmail.com>
// Copyright (c) 2013-2014 Cesanta Software Limited
// All rights reserved
//
// To run this test, start a terminal and execute "make js"

function test(condition, msg) {
  if (!condition) {
    print(msg, ' test failed\n');
    exit(1);
  }
}


var v1 = 123, v2 = 234;
function f(v1) { return v1 ? v1 : v2; }
test(f(1) == 1, 'func params 1');
test(f() == 234, 'func params 2');


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
print(module)
test(module.publicProperty === 'I am public!');
//test(module.publicMethod(3) === 15);
//test(module.privilegedMethod(3, 4) === 1020);


test('a' + 'b' == 'ab', 'string concatenation');
test('He who knows, does not speak'.substr(3, 3) == 'who', 'substr 1');
test('He who speaks, does not know'.substr(24) == 'know', 'substr 2');
test('A warrior never worries about his fear'.indexOf('his') == 30, 'indexOf');
test(Math.PI - 3.1415926 < 0.0001, 'Math 1');
test(1.2345 / 0.0 == Infinity, 'Math 2');
test(undefined != undefined, 'undef');
test((function(x,y){ return x * y;})(3, 4) == 12, 'anon function');

print('Congratulations, all tests passed!');
