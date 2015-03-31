function ap(f) {
    return f()
}
(function() {
    var i,a=0;
    for(i=0;i<1000000;i++) {
        a+=ap(function() {
            return i*2;
        });
    }
})()
