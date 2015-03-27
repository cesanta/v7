function oo() {
    var o = {};
    for (var i=0; i<1000; i++) {
        o['a-' + i] = i;
    }
}

for (var i=0; i<1000; i++) oo();

