function f(i) {
    return 2*i;
}
function ap(f) {
    var a = Array.prototype.slice.call(arguments);
    a.shift();
    return f.call(null, a)
}
(function() {
    var i,a=0;
    for(i=0;i<100000;i++) {
        a+=ap(f,i)
    }
})()
