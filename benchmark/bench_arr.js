var j,i,s;
var a=(function(){return arguments})();
for(j=0;j<10;j++) {
    for (i=0;i<10000;i++) {
        a[i]=i;
    }
    s=0
    for (i=0;i<10000;i++) {
        s+=a[i];
    }
}
s
