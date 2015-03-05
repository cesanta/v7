if (typeof console === "object"){
    prn = console.log;
}else if (typeof document === "object"){
    prn = document.writeln;
// }else if (typeof alert === "function"){
    // prn = alert;
}else prn = print;


prn('my XXzzz xxxZZ'.replace(/x/ig, 'm'));

function replacer(str, p1, p2, offset, s){/*prn('ku-ku', str, p1, p2, offset, s);*/ return p2 + ',' + p1;}
 
prn('my XXzzz xxxZZ'.replace(/(X+)(z+)/ig, replacer));
prn('my XXzzz xxxZZ'.replace(/(X+)(z+)/ig, '$2,$1'));
prn('John Smith'.replace(/(John) (Smith)/ig, '$2,$1'));
prn('John Smith'.replace(/(John) (Smith)/ig, function (str, p1, p2, offset, s){prn('ku-ku'); return p2 + ',' + p1;}));
