function ap(i) {
    return 2*i;
}
(function() {
    var i,a=0;
    for(i=0;i<1000000;i++) {
        a+=ap(i);
    }
})()
