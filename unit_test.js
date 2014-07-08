// Copyright (c) 2004-2013 Sergey Lyubka <valenok@gmail.com>
// Copyright (c) 2013-2014 Cesanta Software Limited
// All rights reserved
//
// To run this test, start a terminal and execute this command:
//    make js

function test(condition, msg) {
  if (!condition) {
    print(msg, ' test failed\n');
    exit(1);
  }
}

function factorial(x) {
  return x <= 1 ? 1 : x * factorial(x - 1);
};

var v1 = 123, v2 = 234;
function f(v1) { return v1 ? v1 : v2; }

test(f(1) == 1, 'func params 1');
test(f() == 234, 'func params 2');
test(factorial(5) == 120, 'factorial 1');
test(factorial(-2) == 1, 'factorial 2');
test(factorial(1) == 1, 'factorial 3');
test('a' + 'b' == 'ab', 'string concatenation');

print('Congratulations, all tests passed!\n');
