var g=false;
(function(f) {
    var i,a=0;
    for(i=0;i<1000000;i++) {
        f(i)
        if (g) {
            eval("a=0");
        } else {
            a+=i;
        }
    }
    return a;
})(function(i){g=(i%10000 == 0)})
