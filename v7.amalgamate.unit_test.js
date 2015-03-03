(function(){
	var files_15_5 = [
		'15.5.1.1_A1_T1.js',
		'15.5.1.1_A1_T2.js',
		'15.5.1.1_A1_T3.js',
		'15.5.1.1_A1_T4.js',
		'15.5.1.1_A1_T5.js',
		'15.5.1.1_A1_T6.js',
		'15.5.1.1_A1_T7.js',
		//'15.5.1.1_A1_T8.js', //new Array
		'15.5.1.1_A1_T9.js',
		'15.5.1.1_A1_T10.js',
		'15.5.1.1_A1_T11.js',
		'15.5.1.1_A1_T12.js',
		//'15.5.1.1_A1_T13.js', //Boolean
		'15.5.1.1_A1_T14.js',
		'15.5.1.1_A1_T15.js',
		'15.5.1.1_A1_T16.js',
		'15.5.1.1_A1_T17.js',
		'15.5.1.1_A1_T18.js',
		//'15.5.1.1_A1_T19.js', //new Array ??? //Assertion failed
		'15.5.2.1_A1_T1.js',
		'15.5.2.1_A1_T2.js',
		'15.5.2.1_A1_T3.js',
		'15.5.2.1_A1_T4.js',
		'15.5.2.1_A1_T5.js',
		'15.5.2.1_A1_T6.js',
		//'15.5.2.1_A1_T7.js', //new String({})
		//'15.5.2.1_A1_T8.js', //new String(function(){})
		'15.5.2.1_A1_T9.js',
		//'15.5.2.1_A1_T10.js', //new __FACTORY
		'15.5.2.1_A1_T11.js',
		//'15.5.2.1_A1_T12.js', // throw
		//'15.5.2.1_A1_T13.js', // throw
		'15.5.2.1_A1_T16.js',
		'15.5.2.1_A1_T17.js',
		'15.5.2.1_A1_T18.js',
		//'15.5.2.1_A1_T19.js', //Assertion failed
		'15.5.1.1_A2_T1.js',
		//'15.5.2.1_A2_T1.js', //String.prototype.isPrototypeOf(__str__obj)
		//'15.5.2.1_A2_T2.js', //type error
		//'15.5.2.1_A3.js', //Object.prototype.toString
		'15.5.3_A1.js',
		//'15.5.3_A2_T1.js', //Function.prototype.isPrototypeOf(String)
		//'15.5.3_A2_T2.js', //String.indicator
		//'15.5.3.1_A1.js', //String.hasOwnProperty('prototype')
		//'15.5.3.1_A2.js', //String.hasOwnProperty('prototype')
		//'15.5.3.1_A3.js', //String.hasOwnProperty('prototype')
		//'15.5.3.1_A4.js', //String.hasOwnProperty('prototype')
		//'15.5.3.2_A1.js', //String.hasOwnProperty("fromCharCode")
		'15.5.3.2_A2.js',
		//'15.5.3.2_A3_T1.js', //String.fromCharCode
		//'15.5.3.2_A3_T2.js', //String.fromCharCode
		//'15.5.3.2_A4.js', //new
		//'15.5.4_A1.js', //String.prototype.toString()
		'15.5.4_A2.js',
		//'15.5.4_A3.js', //Object.prototype.isPrototypeOf(String.prototype)
		'15.5.4.1_A1_T1.js',
		//'15.5.4.1_A1_T2.js', //String.prototype.constructor
		'15.5.4.2_A1_T1.js',
		'15.5.4.2_A1_T2.js',
		'15.5.4.2_A1_T3.js',
		'15.5.4.2_A1_T4.js',
		//'15.5.4.2_A2_T1.js', //String.prototype.toString
		//'15.5.4.2_A2_T2.js', //String.prototype.toString
		//'15.5.4.2_A3_T1.js', //str.valueOf() == str.toString()
		//'15.5.4.2_A4_T1.js', //String.prototype.toString.hasOwnProperty('length')
		//'15.5.4.3_A1_T1.js', //__string__obj.valueOf()
		//'15.5.4.3_A1_T2.js', //__string__obj.valueOf()
		//'15.5.4.3_A1_T3.js', //__string__obj.valueOf()
		//'15.5.4.3_A1_T4.js', //__string__obj.valueOf()
		//'15.5.4.3_A2_T1.js', //String.prototype.valueOf
		//'15.5.4.3_A2_T2.js', //String.prototype.valueOf
		//'15.5.4.4_A1.1.js', //new
		//'15.5.4.4_A1_T1.js', //Assertion failed
		//'15.5.4.4_A1_T2.js', //String.prototype.charAt
		'15.5.4.4_A1_T4.js',
		'15.5.4.4_A1_T5.js',
		'15.5.4.4_A1_T6.js',
		'15.5.4.4_A1_T7.js',
		'15.5.4.4_A1_T8.js',
		'15.5.4.4_A1_T9.js',
		//'15.5.4.4_A1_T10.js', //with
		//'15.5.4.4_A2.js', //String.prototype.charAt
		//'15.5.4.4_A3.js', //__instance.charAt(3) 
		'15.5.4.4_A4_T1.js',
		'15.5.4.4_A4_T2.js',
		'15.5.4.4_A4_T3.js',
		//'15.5.4.4_A5.js', //throw
		'15.5.4.4_A6.js',
		//'15.5.4.4_A7.js', //String.prototype.charAt
		//'15.5.4.4_A8.js', //String.prototype.charAt.hasOwnProperty('length')
		//'15.5.4.4_A9.js', //String.prototype.charAt.hasOwnProperty('length')
		//'15.5.4.4_A10.js', //String.prototype.charAt.hasOwnProperty('length')
		//'15.5.4.4_A11.js', //String.prototype.charAt.hasOwnProperty('length')
		//'15.5.4.5_A1.1.js', //new
		//'15.5.4.5_A1_T1.js', //Assertion failed
		//'15.5.4.5_A1_T2.js', //String.prototype.charCodeAt
		'15.5.4.5_A1_T4.js',
		'15.5.4.5_A1_T5.js',
		'15.5.4.5_A1_T6.js',
		'15.5.4.5_A1_T7.js',
		'15.5.4.5_A1_T8.js',
		'15.5.4.5_A1_T9.js',
		//'15.5.4.5_A1_T10.js', //with
		//'15.5.4.5_A2.js', //String.prototype.charCodeAt
		//'15.5.4.5_A3.js', //__instance.charCodeAt(3)
		//'15.5.4.5_A4.js', // syntax error -- toString:function(){throw 'intostring'},
		'15.5.4.5_A6.js',
		//'15.5.4.5_A7.js', //new
		//'15.5.4.5_A8.js', //String.prototype.charAt.hasOwnProperty('length')
		//'15.5.4.5_A9.js', //String.prototype.charAt.hasOwnProperty('length')
		//'15.5.4.5_A10.js', //String.prototype.charAt.hasOwnProperty('length')
		//'15.5.4.5_A11.js', //String.prototype.charAt.hasOwnProperty('length')
		//'15.5.4.6_A1_T1.js', //Assertion failed
		//'15.5.4.6_A1_T2.js', //String.prototype.concat
		'15.5.4.6_A1_T4.js',
		'15.5.4.6_A1_T5.js',
		'15.5.4.6_A1_T6.js',
		'15.5.4.6_A1_T7.js',
		'15.5.4.6_A1_T8.js',
		'15.5.4.6_A1_T9.js',
		//'15.5.4.6_A1_T10.js', //with
		//'15.5.4.6_A2.js', //Assertion failed
		//'15.5.4.6_A3.js', //__instance.concat("two")
		//'15.5.4.6_A4_T1.js', //String.prototype.concat
		//'15.5.4.6_A4_T2.js', //throw
		'15.5.4.6_A6.js',
		//'15.5.4.6_A7.js', //new
		//'15.5.4.6_A8.js', //String.prototype.concat.hasOwnProperty('length')
		//'15.5.4.6_A9.js', //String.prototype.concat.hasOwnProperty('length')
		//'15.5.4.6_A10.js', //String.prototype.concat.hasOwnProperty('length')
		//'15.5.4.6_A11.js', //String.prototype.concat.hasOwnProperty('length')
		//'15.5.4.7_A1_T1.js', //Assertion failed
		//'15.5.4.7_A1_T2.js', //String.prototype.indexOf
		'15.5.4.7_A1_T4.js',
		'15.5.4.7_A1_T5.js',
		'15.5.4.7_A1_T6.js',
		'15.5.4.7_A1_T7.js',
		'15.5.4.7_A1_T8.js',
		//'15.5.4.7_A1_T9.js', //new
		//'15.5.4.7_A1_T10.js', //with
		//'15.5.4.7_A1_T11.js', //Assertion failed
		//'15.5.4.7_A1_T12.js', //Assertion failed
		'15.5.4.7_A2_T1.js',
		'15.5.4.7_A2_T2.js',
		'15.5.4.7_A2_T3.js',
		'15.5.4.7_A2_T4.js',
		'15.5.4.7_A3_T1.js',
		//'15.5.4.7_A3_T2.js', // syntax error
		'15.5.4.7_A3_T3.js',
		//'15.5.4.7_A4_T1.js', //throw
		//'15.5.4.7_A4_T2.js', //throw
		'15.5.4.7_A4_T3.js',
		//'15.5.4.7_A4_T4.js', //throw
		//'15.5.4.7_A4_T5.js', //throw
		//'15.5.4.7_A5_T1.js', // syntax error
		//'15.5.4.7_A5_T2.js', // syntax error
		//'15.5.4.7_A5_T3.js', // syntax error
		//'15.5.4.7_A5_T4.js', // syntax error
		//'15.5.4.7_A5_T5.js', // syntax error
		//'15.5.4.7_A5_T6.js', // syntax error
		'15.5.4.7_A6.js',
		//'15.5.4.7_A7.js', //new
		//'15.5.4.7_A8.js', //String.prototype.indexOf.hasOwnProperty('length')
		//'15.5.4.7_A9.js', //String.prototype.indexOf.hasOwnProperty('length')
		//'15.5.4.7_A10.js', //String.prototype.indexOf.hasOwnProperty('length')
		//'15.5.4.7_A11.js', //String.prototype.indexOf.hasOwnProperty('length')
		//'15.5.4.8_A1_T1.js', //Assertion failed
		//'15.5.4.8_A1_T2.js', //String.prototype.lastIndexOf
		'15.5.4.8_A1_T4.js',
		'15.5.4.8_A1_T5.js',
		'15.5.4.8_A1_T6.js',
		'15.5.4.8_A1_T7.js',
		'15.5.4.8_A1_T8.js',
		//'15.5.4.8_A1_T9.js', //new
		//'15.5.4.8_A1_T10.js', //with
		//'15.5.4.8_A1_T12.js', //Assertion failed
		//'15.5.4.8_A4_T1.js', //throw
		//'15.5.4.8_A4_T2.js', //throw
		'15.5.4.8_A4_T3.js',
		//'15.5.4.8_A4_T4.js', //throw
		//'15.5.4.8_A4_T5.js', //throw
		'15.5.4.8_A6.js',
		//'15.5.4.8_A7.js', //new
		//'15.5.4.8_A8.js', //String.prototype.lastIndexOf.hasOwnProperty('length')
		//'15.5.4.8_A9.js', //String.prototype.lastIndexOf.hasOwnProperty('length')
		//'15.5.4.8_A10.js', //String.prototype.lastIndexOf.hasOwnProperty('length')
		//'15.5.4.8_A11.js', //String.prototype.lastIndexOf.hasOwnProperty('length')
		'15.5.4.9_3.js',
		//'15.5.4.9_A1_T1.js', //str1.localeCompare(str2)
		'15.5.4.9_A1_T2.js',
		'15.5.4.9_A6.js',
		//'15.5.4.9_A7.js', //new
		//'15.5.4.9_A8.js', //String.prototype.localeCompare.hasOwnProperty('length')
		//'15.5.4.9_A9.js', //String.prototype.localeCompare.hasOwnProperty('length')
		//'15.5.4.9_A10.js', //String.prototype.localeCompare.hasOwnProperty('length')
		//'15.5.4.9_A11.js', //String.prototype.localeCompare.hasOwnProperty('length')
		'15.5.4.9_CE.js', //for
		//'15.5.4.10_A1_T1.js', //Assertion failed
		//'15.5.4.10_A1_T2.js', //String.prototype.match
		//'15.5.4.10_A1_T3.js', //String.prototype.match.bind(fnGlobalObject())
		//'15.5.4.10_A1_T4.js', //RegExp().exec("")
		'15.5.4.10_A1_T5.js',
		'15.5.4.10_A1_T6.js',
		'15.5.4.10_A1_T7.js', //for
		'15.5.4.10_A1_T8.js', //for
		//'15.5.4.10_A1_T9.js', //new
		//'15.5.4.10_A1_T10.js', //with
		//'15.5.4.10_A1_T11.js', //throw
		//'15.5.4.10_A1_T12.js', //throw
		//'15.5.4.10_A1_T13.js', //??????????????????????????
		//'15.5.4.10_A1_T14.js', //Assertion failed
		'15.5.4.10_A2_T1.js',
		//'15.5.4.10_A2_T2.js', //??????????????????????????
		//'15.5.4.10_A2_T3.js', //syntax error -- if (__string.match(/\d{1}/g)[mi]!==__matches[mi]) {
		//'15.5.4.10_A2_T4.js', //syntax error
		//'15.5.4.10_A2_T5.js', //??????????????????????????
		//'15.5.4.10_A2_T6.js', //syntax error
		//'15.5.4.10_A2_T7.js', //syntax error
		'15.5.4.10_A2_T8.js', //for
		'15.5.4.10_A2_T9.js', //for
		'15.5.4.10_A2_T10.js', //for
		'15.5.4.10_A2_T11.js', //for
		'15.5.4.10_A2_T12.js',
		'15.5.4.10_A2_T13.js',
		'15.5.4.10_A2_T14.js',
		'15.5.4.10_A2_T15.js',
		'15.5.4.10_A2_T16.js',
		//'15.5.4.10_A2_T17.js', //String.prototype.match
		//'15.5.4.10_A2_T18.js', //String.prototype.match
		'15.5.4.10_A6.js',
		//'15.5.4.10_A7.js', //new
		//'15.5.4.10_A8.js', //String.prototype.match.hasOwnProperty('length')
		//'15.5.4.10_A9.js', //String.prototype.match.hasOwnProperty('length')
		//'15.5.4.10_A10.js', //String.prototype.match.hasOwnProperty('length')
		//'15.5.4.10_A11.js', //String.prototype.match.hasOwnProperty('length')
		//'15.5.4.11_A1_T1.js', //Assertion failed
		//'15.5.4.11_A1_T2.js', //String.prototype.replace
		'15.5.4.11_A1_T4.js',
		//'15.5.4.11_A1_T5.js', //??????????????????????????
		//'15.5.4.11_A1_T6.js', //??????????????????????????
		'15.5.4.11_A1_T7.js',
		//'15.5.4.11_A1_T8.js', //??????????????????????????
		//'15.5.4.11_A1_T9.js', //new
		//'15.5.4.11_A1_T10.js', //with
		//'15.5.4.11_A1_T11.js', //throw
		//'15.5.4.11_A1_T12.js', //throw
		//'15.5.4.11_A1_T13.js', //throw
		//'15.5.4.11_A1_T14.js', //Assertion failed
		//'15.5.4.11_A1_T15.js', //syntax error -- {toString:function(){return /77/}};
		//'15.5.4.11_A1_T16.js', //Assertion failed
		//'15.5.4.11_A1_T17.js', //Assertion failed
		'15.5.4.11_A2_T1.js',
		'15.5.4.11_A2_T2.js',
		'15.5.4.11_A2_T3.js',
		'15.5.4.11_A2_T4.js',
		'15.5.4.11_A2_T5.js',
		'15.5.4.11_A2_T6.js',
		'15.5.4.11_A2_T7.js',
		'15.5.4.11_A2_T8.js',
		'15.5.4.11_A2_T9.js',
		'15.5.4.11_A2_T10.js',
		'15.5.4.11_A3_T1.js',
		'15.5.4.11_A3_T2.js',
		'15.5.4.11_A3_T3.js',
		'15.5.4.11_A4_T1.js',
		'15.5.4.11_A4_T2.js',
		'15.5.4.11_A4_T3.js',
		'15.5.4.11_A4_T4.js',
		'15.5.4.11_A5_T1.js',
		'15.5.4.11_A6.js',
		//'15.5.4.11_A7.js', //new
		//'15.5.4.11_A8.js', //String.prototype.replace.hasOwnProperty('length')
		//'15.5.4.11_A9.js', //String.prototype.replace.hasOwnProperty('length')
		//'15.5.4.11_A10.js', //String.prototype.replace.hasOwnProperty('length')
		//'15.5.4.11_A11.js', //String.prototype.replace.hasOwnProperty('length')
		//'15.5.4.11_A12.js', //Assertion failed
		//'15.5.4.11-1.js', //syntax error -- function()
		'15.5.4.12_A1.1_T1.js',
		//'15.5.4.12_A1_T1.js', //Assertion failed
		//'15.5.4.12_A1_T2.js', //String.prototype.search
		'15.5.4.12_A1_T4.js',
		'15.5.4.12_A1_T5.js',
		'15.5.4.12_A1_T6.js',
		'15.5.4.12_A1_T7.js',
		'15.5.4.12_A1_T8.js',
		//'15.5.4.12_A1_T9.js', //new
		//'15.5.4.12_A1_T10.js', //with
		//'15.5.4.12_A1_T11.js', //throw
		//'15.5.4.12_A1_T12.js', //throw
		//'15.5.4.12_A1_T13.js', //??????????????????????????
		//'15.5.4.12_A1_T14.js', //Assertion failed
		//'15.5.4.12_A2_T1.js', //???????????????????????????
		//'15.5.4.12_A2_T2.js', //???????????????????????????
		//'15.5.4.12_A2_T3.js', //???????????????????????????
		//'15.5.4.12_A2_T4.js', //???????????????????????????
		//'15.5.4.12_A2_T5.js', //???????????????????????????
		//'15.5.4.12_A2_T6.js', //???????????????????????????
		//'15.5.4.12_A2_T7.js', //???????????????????????????
		//'15.5.4.12_A3_T1.js', //???????????????????????????
		//'15.5.4.12_A3_T2.js', //???????????????????????????
		'15.5.4.12_A6.js',
		//'15.5.4.12_A7.js', //new
		//'15.5.4.12_A8.js', //String.prototype.search.hasOwnProperty('length')
		//'15.5.4.12_A9.js', //String.prototype.search.hasOwnProperty('length')
		//'15.5.4.12_A10.js', //String.prototype.search.hasOwnProperty('length')
		//'15.5.4.12_A11.js', //String.prototype.search.hasOwnProperty('length')
		//'15.5.4.13_A1_T1.js', //Assertion failed
		//'15.5.4.13_A1_T2.js', //String.prototype.slice
		'15.5.4.13_A1_T4.js',
		//'15.5.4.13_A1_T5.js', //type error -- __func.toString=function(){return __func;};
		'15.5.4.13_A1_T6.js',
		'15.5.4.13_A1_T7.js',
		'15.5.4.13_A1_T8.js',
		//'15.5.4.13_A1_T9.js', //new
		//'15.5.4.13_A1_T10.js', //with
		//'15.5.4.13_A1_T11.js', //throw
		//'15.5.4.13_A1_T12.js', //throw
		//'15.5.4.13_A1_T13.js', //throw
		'15.5.4.13_A1_T14.js',
		//'15.5.4.13_A1_T15.js', //type error -- if (__num.slice()!=="11.001002") {
		//'15.5.4.13_A2_T1.js', //???????????????????????????????
		//'15.5.4.13_A2_T2.js', //????????????????????????????
		//'15.5.4.13_A2_T3.js', //???????????????????????
		//'15.5.4.13_A2_T4.js', //???????????????????????????????
		//'15.5.4.13_A2_T5.js', //???????????????????????
		//'15.5.4.13_A2_T6.js', //????????????????????
		//'15.5.4.13_A2_T7.js', //????????????????????????
		//'15.5.4.13_A2_T8.js', //??????????????????????????
		//'15.5.4.13_A2_T9.js', //??????????????????????
		//'15.5.4.13_A3_T1.js', //Assertion failed
		//'15.5.4.13_A3_T2.js', //Assertion failed
		//'15.5.4.13_A3_T3.js', //String.prototype.slice
		//'15.5.4.13_A3_T4.js', //type error
		'15.5.4.13_A6.js',
		//'15.5.4.13_A7.js', //String.prototype.slice
		//'15.5.4.13_A8.js', //String.prototype.slice.hasOwnProperty('length')
		//'15.5.4.13_A9.js', //String.prototype.slice.hasOwnProperty('length')
		//'15.5.4.13_A10.js', //String.prototype.slice.hasOwnProperty('length')
		//'15.5.4.13_A11.js', //String.prototype.slice.hasOwnProperty('length')
		//'15.5.4.14_A1_T1.js', //Assertion failed
		//'15.5.4.14_A1_T2.js', //String.prototype.split
		//'15.5.4.14_A1_T3.js', //String.prototype.split.bind(this)
		'15.5.4.14_A1_T4.js',
		'15.5.4.14_A1_T5.js',
		'15.5.4.14_A1_T6.js',
		'15.5.4.14_A1_T7.js',
		'15.5.4.14_A1_T8.js',
		//'15.5.4.14_A1_T9.js', //new
		//'15.5.4.14_A1_T10.js', //with
		//'15.5.4.14_A1_T11.js', //throw
		//'15.5.4.14_A1_T12.js', //throw
		'15.5.4.14_A1_T13.js',
		//'15.5.4.14_A1_T14.js', //throw
		//'15.5.4.14_A1_T15.js', //throw
		//'15.5.4.14_A1_T16.js', //{toString:function(){return /\u0037\u0037/g;}}
		//'15.5.4.14_A1_T17.js', //String.prototype.split
		//'15.5.4.14_A1_T18.js', //Assertion failed
		//'15.5.4.14_A2_T2.js', //????????????????????
		//'15.5.4.14_A2_T3.js', //????????????????????????
		//'15.5.4.14_A2_T4.js', //?????????????????????
		//'15.5.4.14_A2_T5.js', //????????????????????????
		//'15.5.4.14_A2_T6.js', //????????????????????????????
		'15.5.4.14_A2_T7.js', //for
		'15.5.4.14_A2_T8.js', //for
		'15.5.4.14_A2_T9.js', //for
		//'15.5.4.14_A2_T1.js', //?????????????????????????????
		'15.5.4.14_A2_T10.js', //for
		//'15.5.4.14_A2_T11.js', //??????????????????????????
		//'15.5.4.14_A2_T12.js', //????????????????????????
		//'15.5.4.14_A2_T13.js', //??????????????????????????
		//'15.5.4.14_A2_T14.js', //?????????????????????
		//'15.5.4.14_A2_T15.js', //?????????????????????????
		//'15.5.4.14_A2_T16.js', //???????????????????????
		//'15.5.4.14_A2_T17.js', //?????????????????????????
		//'15.5.4.14_A2_T18.js', //????????????????????????????
		'15.5.4.14_A2_T19.js',
		//'15.5.4.14_A2_T20.js', //???????????????????????????
		//'15.5.4.14_A2_T21.js', //?????????????????????????
		//'15.5.4.14_A2_T22.js', //?????????????????????????????
		//'15.5.4.14_A2_T23.js', //??????????????????????????????
		//'15.5.4.14_A2_T24.js', //??????????????????????????
		//'15.5.4.14_A2_T25.js', //?????????????????????????
		//'15.5.4.14_A2_T26.js', //???????????????????????????
		//'15.5.4.14_A2_T27.js', //?????????????????????????????
		//'15.5.4.14_A2_T28.js', //??????????????????????????????
		//'15.5.4.14_A2_T29.js', //Assertion failed
		//'15.5.4.14_A2_T30.js', //Assertion failed
		//'15.5.4.14_A2_T31.js', //Assertion failed
		//'15.5.4.14_A2_T32.js', //Assertion failed
		//'15.5.4.14_A2_T33.js', //Assertion failed
		//'15.5.4.14_A2_T34.js', //Assertion failed
		//'15.5.4.14_A2_T35.js', //Assertion failed
		//'15.5.4.14_A2_T36.js', //Assertion failed
		//'15.5.4.14_A2_T37.js', //Assertion failed
		//'15.5.4.14_A2_T38.js', //????????????????????????????
		//'15.5.4.14_A2_T39.js', //???????????????????????????
		//'15.5.4.14_A2_T40.js', //????????????????????????????
		//'15.5.4.14_A2_T41.js', //???????????????????????????
		//'15.5.4.14_A2_T42.js', //????????????????????????????
		//'15.5.4.14_A2_T43.js', //???????????????????????????
		//'15.5.4.14_A3_T1.js', //???????????????
		//'15.5.4.14_A3_T2.js', //Assertion failed
		//'15.5.4.14_A3_T3.js', //String.prototype.split
		//'15.5.4.14_A3_T4.js', //Assertion failed
		//'15.5.4.14_A3_T5.js', //Assertion failed
		//'15.5.4.14_A3_T6.js', //Assertion failed
		//'15.5.4.14_A3_T7.js', //String.prototype.split
		//'15.5.4.14_A3_T8.js', //Assertion failed
		//'15.5.4.14_A3_T9.js', //String.prototype.split
		//'15.5.4.14_A3_T10.js', //split()
		//'15.5.4.14_A3_T11.js', //?????????????????????
		//'15.5.4.14_A4_T1.js', //?????????????????????
		//'15.5.4.14_A4_T2.js', //????????????????????????????
		//'15.5.4.14_A4_T3.js', //?????????????????????????
		//'15.5.4.14_A4_T4.js', //??????????????????????????
		//'15.5.4.14_A4_T5.js', //???????????????????????
		//'15.5.4.14_A4_T6.js', //?????????????????????????
		//'15.5.4.14_A4_T7.js', //??????????????????????????
		//'15.5.4.14_A4_T8.js', //?????????????????????
		//'15.5.4.14_A4_T9.js', //?????????????????????????
		//'15.5.4.14_A4_T10.js', //???????????????????????
		//'15.5.4.14_A4_T11.js', //??????????????????????
		//'15.5.4.14_A4_T12.js', //???????????????????????????
		//'15.5.4.14_A4_T13.js', //???????????????????????????
		//'15.5.4.14_A4_T14.js', //?????????????????????????
		//'15.5.4.14_A4_T15.js', //???????????????????????
		//'15.5.4.14_A4_T16.js', //????????????????????????
		//'15.5.4.14_A4_T17.js', //????????????????????????
		//'15.5.4.14_A4_T18.js', //??????????????????????????
		//'15.5.4.14_A4_T19.js', //???????????????????????????
		//'15.5.4.14_A4_T20.js', //?????????????????????????
		//'15.5.4.14_A4_T21.js', //???????????????????????
		//'15.5.4.14_A4_T22.js', //??????????????????????
		//'15.5.4.14_A4_T23.js', //Assertion failed
		//'15.5.4.14_A4_T24.js', //?????????????????????
		//'15.5.4.14_A4_T25.js', //Assertion failed
		'15.5.4.14_A6.js',
		//'15.5.4.14_A7.js', //new
		//'15.5.4.14_A8.js', //String.prototype.split.hasOwnProperty('length')
		//'15.5.4.14_A9.js', //String.prototype.split.hasOwnProperty('length')
		//'15.5.4.14_A10.js', //String.prototype.split.hasOwnProperty('length')
		//'15.5.4.14_A11.js', //String.prototype.split.hasOwnProperty('length')
		//'15.5.4.15_A1_T1.js', //Assertion failed
		//'15.5.4.15_A1_T2.js', //String.prototype.substring
		'15.5.4.15_A1_T4.js',
		//'15.5.4.15_A1_T5.js', //String.prototype.substring
		'15.5.4.15_A1_T6.js',
		'15.5.4.15_A1_T7.js',
		'15.5.4.15_A1_T8.js',
		//'15.5.4.15_A1_T9.js', //new
		//'15.5.4.15_A1_T10.js', //with
		//'15.5.4.15_A1_T11.js', //throw
		//'15.5.4.15_A1_T12.js', //throw
		//'15.5.4.15_A1_T13.js', //throw
		'15.5.4.15_A1_T14.js',
		//'15.5.4.15_A1_T15.js', //String.prototype.substring
		//'15.5.4.15_A2_T1.js', //?????????????????
		//'15.5.4.15_A2_T2.js', //?????????????????????
		//'15.5.4.15_A2_T3.js', //??????????????????????
		//'15.5.4.15_A2_T4.js', //????????????????????
		//'15.5.4.15_A2_T5.js', //?????????????????????????
		//'15.5.4.15_A2_T6.js', //???????????????????????
		//'15.5.4.15_A2_T7.js', //?????????????????????
		//'15.5.4.15_A2_T8.js', //???????????????????
		//'15.5.4.15_A2_T9.js', //???????????????????
		//'15.5.4.15_A2_T10.js', //???????????????????
		//'15.5.4.15_A3_T1.js', //Assertion failed
		//'15.5.4.15_A3_T2.js', //Assertion failed
		//'15.5.4.15_A3_T3.js', //Assertion failed
		//'15.5.4.15_A3_T4.js', //Assertion failed
		//'15.5.4.15_A3_T5.js', //Assertion failed
		//'15.5.4.15_A3_T6.js', //Assertion failed
		//'15.5.4.15_A3_T7.js', //String.prototype.substring
		//'15.5.4.15_A3_T8.js', //Assertion failed
		//'15.5.4.15_A3_T9.js', //String.prototype.substring
		//'15.5.4.15_A3_T10.js', //new
		//'15.5.4.15_A3_T11.js', //new
		'15.5.4.15_A6.js',
		//'15.5.4.15_A7.js', //new
		//'15.5.4.15_A8.js', //String.prototype.substring.hasOwnProperty('length')
		//'15.5.4.15_A9.js', //String.prototype.substring.hasOwnProperty('length')
		//'15.5.4.15_A10.js', //String.prototype.substring.hasOwnProperty('length')
		//'15.5.4.15_A11.js', //String.prototype.substring.hasOwnProperty('length')
		//'15.5.4.16_A1_T1.js', //Assertion failed
		//'15.5.4.16_A1_T2.js', //String.prototype.toLowerCase
		//'15.5.4.16_A1_T3.js', //if (eval("\"BJ\"").toLowerCase() !== "bj") {
		'15.5.4.16_A1_T4.js',
		'15.5.4.16_A1_T5.js',
		//'15.5.4.16_A1_T6.js', //String.prototype.toLowerCase
		//'15.5.4.16_A1_T7.js', //String.prototype.toLowerCase
		//'15.5.4.16_A1_T8.js', //String.prototype.toLowerCase
		//'15.5.4.16_A1_T9.js', //new
		//'15.5.4.16_A1_T10.js', //syntax error
		//'15.5.4.16_A1_T11.js', //throw
		//'15.5.4.16_A1_T12.js', //throw
		//'15.5.4.16_A1_T13.js', //syntax error
		//'15.5.4.16_A1_T14.js', //Assertion failed
		'15.5.4.16_A2_T1.js',
		'15.5.4.16_A6.js',
		//'15.5.4.16_A7.js', //new
		//'15.5.4.16_A8.js', //String.prototype.toLowerCase.hasOwnProperty('length')
		//'15.5.4.16_A9.js', //String.prototype.toLowerCase.hasOwnProperty('length')
		//'15.5.4.16_A10.js', //String.prototype.toLowerCase.hasOwnProperty('length')
		//'15.5.4.16_A11.js', //String.prototype.toLowerCase.hasOwnProperty('length')
		//'15.5.4.17_A1_T1.js', //Assertion failed
		//'15.5.4.17_A1_T2.js', //String.prototype.toLocaleLowerCase
		//'15.5.4.17_A1_T3.js', //if (eval("\"BJ\"").toLocaleLowerCase() !== "bj") {
		'15.5.4.17_A1_T4.js',
		'15.5.4.17_A1_T5.js',
		//'15.5.4.17_A1_T6.js', //String.prototype.toLocaleLowerCase
		//'15.5.4.17_A1_T7.js', //String.prototype.toLocaleLowerCase
		//'15.5.4.17_A1_T8.js', //String.prototype.toLocaleLowerCase
		//'15.5.4.17_A1_T9.js', //new
		//'15.5.4.17_A1_T10.js', //String.prototype.toLocaleLowerCase
		//'15.5.4.17_A1_T11.js', //throw
		//'15.5.4.17_A1_T12.js', //throw
		//'15.5.4.17_A1_T13.js', //String.prototype.toLocaleLowerCase
		//'15.5.4.17_A1_T14.js', //Assertion failed
		'15.5.4.17_A2_T1.js',
		'15.5.4.17_A6.js',
		//'15.5.4.17_A7.js', //new
		//'15.5.4.17_A8.js', //String.prototype.toLocaleLowerCase.hasOwnProperty('length')
		//'15.5.4.17_A9.js', //String.prototype.toLocaleLowerCase.hasOwnProperty('length')
		//'15.5.4.17_A10.js', //String.prototype.toLocaleLowerCase.hasOwnProperty('length')
		//'15.5.4.17_A11.js', //String.prototype.toLocaleLowerCase.hasOwnProperty('length')
		//'15.5.4.18_A1_T1.js', //Assertion failed
		//'15.5.4.18_A1_T2.js', //String.prototype.toUpperCase
		//'15.5.4.18_A1_T3.js', //if (eval("\"bj\"").toUpperCase() !== "BJ") {
		'15.5.4.18_A1_T4.js',
		'15.5.4.18_A1_T5.js',
		//'15.5.4.18_A1_T6.js', //String.prototype.toUpperCase
		//'15.5.4.18_A1_T7.js', //String.prototype.toUpperCase
		//'15.5.4.18_A1_T8.js', //String.prototype.toUpperCase
		//'15.5.4.18_A1_T9.js', //new
		//'15.5.4.18_A1_T10.js', //String.prototype.toUpperCase
		//'15.5.4.18_A1_T11.js', //throw
		//'15.5.4.18_A1_T12.js', //throw
		//'15.5.4.18_A1_T13.js', //String.prototype.toUpperCase
		//'15.5.4.18_A1_T14.js', //Assertion failed
		'15.5.4.18_A2_T1.js',
		'15.5.4.18_A6.js',
		//'15.5.4.18_A7.js', //new
		//'15.5.4.18_A8.js', //String.prototype.toUpperCase.hasOwnProperty('length')
		//'15.5.4.18_A9.js', //String.prototype.toUpperCase.hasOwnProperty('length')
		//'15.5.4.18_A10.js', //String.prototype.toUpperCase.hasOwnProperty('length')
		//'15.5.4.18_A11.js', //String.prototype.toUpperCase.hasOwnProperty('length')
		//'15.5.4.19_A1_T1.js', //Assertion failed
		//'15.5.4.19_A1_T2.js', //String.prototype.toLocaleUpperCase
		//'15.5.4.19_A1_T3.js', //if (eval("\"bj\"").toLocaleUpperCase() !== "BJ") {
		'15.5.4.19_A1_T4.js',
		'15.5.4.19_A1_T5.js',
		//'15.5.4.19_A1_T6.js', //String.prototype.toLocaleUpperCase
		//'15.5.4.19_A1_T7.js', //String.prototype.toLocaleUpperCase
		//'15.5.4.19_A1_T8.js', //String.prototype.toLocaleUpperCase
		//'15.5.4.19_A1_T9.js', //new
		//'15.5.4.19_A1_T10.js', //String.prototype.toLocaleUpperCase
		//'15.5.4.19_A1_T11.js', //throw
		//'15.5.4.19_A1_T12.js', //throw
		//'15.5.4.19_A1_T13.js', //String.prototype.toLocaleUpperCase
		//'15.5.4.19_A1_T14.js', //Assertion failed
		'15.5.4.19_A2_T1.js',
		'15.5.4.19_A6.js',
		//'15.5.4.19_A7.js', //new
		//'15.5.4.19_A8.js', //String.prototype.toLocaleUpperCase.hasOwnProperty('length')
		//'15.5.4.19_A9.js', //String.prototype.toLocaleUpperCase.hasOwnProperty('length')
		//'15.5.4.19_A10.js', //String.prototype.toLocaleUpperCase.hasOwnProperty('length')
		//'15.5.4.19_A11.js', //String.prototype.toLocaleUpperCase.hasOwnProperty('length')
		'15.5.4.20-0-1.js',
		'15.5.4.20-0-2.js',
		//'15.5.4.20-1-1.js', //catch
		//'15.5.4.20-1-2.js', //catch
		//'15.5.4.20-1-3.js', //catch
		//'15.5.4.20-1-4.js', //catch
		//'15.5.4.20-1-5.js', //catch
		//'15.5.4.20-1-6.js', //catch
		//'15.5.4.20-1-7.js', //catch
		'15.5.4.20-1-8.js',
		//'15.5.4.20-1-9.js', //trim()
		//'15.5.4.20-2-1.js', //String.prototype.trim.call
		//'15.5.4.20-2-2.js', //String.prototype.trim.call
		//'15.5.4.20-2-3.js', //String.prototype.trim.call
		//'15.5.4.20-2-4.js', //String.prototype.trim.call
		//'15.5.4.20-2-5.js', //String.prototype.trim.call
		//'15.5.4.20-2-6.js', //String.prototype.trim.call
		//'15.5.4.20-2-7.js', //String.prototype.trim.call
		//'15.5.4.20-2-8.js', //String.prototype.trim.call
		//'15.5.4.20-2-9.js', //String.prototype.trim.call
		//'15.5.4.20-2-10.js', //String.prototype.trim.call
		//'15.5.4.20-2-11.js', //String.prototype.trim.call
		//'15.5.4.20-2-12.js', //String.prototype.trim.call
		//'15.5.4.20-2-13.js', //String.prototype.trim.call
		//'15.5.4.20-2-14.js', //String.prototype.trim.call
		//'15.5.4.20-2-15.js', //String.prototype.trim.call
		//'15.5.4.20-2-16.js', //String.prototype.trim.call
		//'15.5.4.20-2-17.js', //String.prototype.trim.call
		//'15.5.4.20-2-18.js', //String.prototype.trim.call
		//'15.5.4.20-2-19.js', //String.prototype.trim.call
		//'15.5.4.20-2-20.js', //String.prototype.trim.call
		//'15.5.4.20-2-21.js', //String.prototype.trim.call
		//'15.5.4.20-2-22.js', //String.prototype.trim.call
		//'15.5.4.20-2-23.js', //String.prototype.trim.call
		//'15.5.4.20-2-24.js', //String.prototype.trim.call
		//'15.5.4.20-2-25.js', //String.prototype.trim.call
		//'15.5.4.20-2-26.js', //String.prototype.trim.call
		//'15.5.4.20-2-27.js', //String.prototype.trim.call
		//'15.5.4.20-2-28.js', //String.prototype.trim.call
		//'15.5.4.20-2-29.js',
		//'15.5.4.20-2-30.js',
		//'15.5.4.20-2-31.js',
		//'15.5.4.20-2-32.js',
		//'15.5.4.20-2-33.js',
		//'15.5.4.20-2-34.js',
		//'15.5.4.20-2-35.js',
		//'15.5.4.20-2-36.js',
		//'15.5.4.20-2-37.js',
		//'15.5.4.20-2-38.js',
		//'15.5.4.20-2-39.js',
		//'15.5.4.20-2-40.js',
		//'15.5.4.20-2-41.js',
		//'15.5.4.20-2-42.js',
		//'15.5.4.20-2-43.js',
		//'15.5.4.20-2-44.js',
		//'15.5.4.20-2-45.js',
		//'15.5.4.20-2-46.js',
		//'15.5.4.20-2-47.js',
		//'15.5.4.20-2-49.js',
		//'15.5.4.20-2-50.js', //String.prototype.trim.call
		//'15.5.4.20-2-51.js', //String.prototype.trim.call
		'15.5.4.20-3-1.js',
		'15.5.4.20-3-2.js',
		'15.5.4.20-3-3.js',
		'15.5.4.20-3-4.js',
		'15.5.4.20-3-5.js',
		'15.5.4.20-3-6.js',
		'15.5.4.20-3-7.js',
		'15.5.4.20-3-8.js',
		'15.5.4.20-3-9.js',
		'15.5.4.20-3-10.js',
		'15.5.4.20-3-11.js',
		'15.5.4.20-3-12.js',
		'15.5.4.20-3-13.js',
		'15.5.4.20-3-14.js',
		'15.5.4.20-4-1.js',
		'15.5.4.20-4-2.js',
		'15.5.4.20-4-3.js',
		'15.5.4.20-4-4.js',
		'15.5.4.20-4-5.js',
		'15.5.4.20-4-6.js',
		'15.5.4.20-4-8.js',
		'15.5.4.20-4-10.js',
		'15.5.4.20-4-11.js',
		'15.5.4.20-4-12.js',
		'15.5.4.20-4-13.js',
		'15.5.4.20-4-14.js',
		'15.5.4.20-4-16.js',
		'15.5.4.20-4-18.js',
		'15.5.4.20-4-19.js',
		'15.5.4.20-4-20.js',
		'15.5.4.20-4-21.js',
		'15.5.4.20-4-22.js',
		'15.5.4.20-4-24.js',
		'15.5.4.20-4-27.js',
		'15.5.4.20-4-28.js',
		'15.5.4.20-4-29.js',
		'15.5.4.20-4-30.js',
		'15.5.4.20-4-32.js',
		'15.5.4.20-4-34.js',
		'15.5.4.20-4-35.js',
		'15.5.4.20-4-36.js',
		'15.5.4.20-4-37.js',
		'15.5.4.20-4-38.js',
		'15.5.4.20-4-39.js',
		'15.5.4.20-4-40.js',
		'15.5.4.20-4-41.js',
		'15.5.4.20-4-42.js',
		'15.5.4.20-4-43.js',
		'15.5.4.20-4-44.js',
		'15.5.4.20-4-45.js',
		'15.5.4.20-4-46.js',
		'15.5.4.20-4-47.js',
		'15.5.4.20-4-48.js',
		'15.5.4.20-4-49.js',
		'15.5.4.20-4-50.js',
		'15.5.4.20-4-51.js',
		'15.5.4.20-4-52.js',
		'15.5.4.20-4-53.js',
		'15.5.4.20-4-54.js',
		'15.5.4.20-4-55.js',
		'15.5.4.20-4-56.js',
		'15.5.4.20-4-57.js',
		'15.5.4.20-4-58.js',
		'15.5.4.20-4-59.js',
		'15.5.4.20-4-60.js',

		//'15.5.5_A1_T1.js', //try
		//'15.5.5_A1_T2.js', //try
		//'15.5.5_A2_T1.js', //Assertion failed
		//'15.5.5_A2_T2.js', //new new
		'15.5.5.1_A1.js',
		//'15.5.5.1_A2.js', //????????????????????
		//'15.5.5.1_A3.js', //?????????????????
		//'15.5.5.1_A4.js', //????????????????
		'15.5.5.1_A5.js',
		'15.5.5.5.2-1-1.js',
		'15.5.5.5.2-1-2.js',
		'15.5.5.5.2-3-1.js',
		'15.5.5.5.2-3-2.js',
		'15.5.5.5.2-3-3.js',
		'15.5.5.5.2-3-4.js',
		'15.5.5.5.2-3-5.js',
		'15.5.5.5.2-3-6.js',
		'15.5.5.5.2-3-7.js',
		'15.5.5.5.2-3-8.js',
		'15.5.5.5.2-7-1.js',
		'15.5.5.5.2-7-2.js',
		'15.5.5.5.2-7-3.js',
		'15.5.5.5.2-7-4.js'

	];

	var files_15_10 = [
		//'15.10.1_A1_T1.js', //Assertion failed
		//'15.10.1_A1_T2.js', //Assertion failed
		//'15.10.1_A1_T3.js', //Assertion failed
		//'15.10.1_A1_T4.js', //Assertion failed
		//'15.10.1_A1_T5.js', //Assertion failed
		//'15.10.1_A1_T6.js', //Assertion failed
		//'15.10.1_A1_T7.js', //Assertion failed
		//'15.10.1_A1_T8.js', //Assertion failed
		//'15.10.1_A1_T9.js', //Assertion failed
		//'15.10.1_A1_T10.js', //Assertion failed
		//'15.10.1_A1_T11.js', //Assertion failed
		//'15.10.1_A1_T12.js', //Assertion failed
		//'15.10.1_A1_T13.js', //Assertion failed
		//'15.10.1_A1_T14.js', //Assertion failed
		//'15.10.1_A1_T15.js', //Assertion failed
		//'15.10.1_A1_T16.js', //Assertion failed
		'15.10.2.10_A1.1_T1.js',
		'15.10.2.10_A1.2_T1.js',
		'15.10.2.10_A1.3_T1.js',
		'15.10.2.10_A1.4_T1.js',
		'15.10.2.10_A1.5_T1.js',
		//'15.10.2.10_A2.1_T1.js', //for -- String.fromCharCode()
		//'15.10.2.10_A2.1_T2.js', //for -- String.fromCharCode()
		//'15.10.2.10_A2.1_T3.js', //for -- String.fromCharCode()
		'15.10.2.10_A3.1_T1.js',
		'15.10.2.10_A3.1_T2.js', //for
		'15.10.2.10_A4.1_T1.js',
		'15.10.2.10_A4.1_T2.js', //for
		'15.10.2.10_A4.1_T3.js', //for
		//'15.10.2.10_A5.1_T1.js', //do
		//'15.10.2.11_A1_T1.js', //Assertion failed
		'15.10.2.11_A1_T4.js',
		//'15.10.2.11_A1_T5.js', //BAD TEST - RegExp error --- /\1(A)/
		'15.10.2.11_A1_T6.js',
		//'15.10.2.11_A1_T7.js', //BAD TEST - RegExp error --- /\1(A)(B)\2/
		'15.10.2.11_A1_T8.js', //for
		'15.10.2.11_A1_T9.js', //for
		//'15.10.2.12_A1_T1.js', //for -- String.fromCharCode()
		'15.10.2.12_A1_T2.js',
		//'15.10.2.12_A1_T3.js', //for -- String.fromCharCode()
		//'15.10.2.12_A1_T4.js', //for -- String.fromCharCode()
		//'15.10.2.12_A1_T5.js', //while -- lastIndex ???
		//'15.10.2.12_A2_T1.js', //for -- String.fromCharCode()
		'15.10.2.12_A2_T2.js',
		//'15.10.2.12_A2_T3.js', //for -- String.fromCharCode()
		//'15.10.2.12_A2_T4.js', //for -- String.fromCharCode()
		//'15.10.2.12_A2_T5.js', //while -- lastIndex ???
		//'15.10.2.12_A3_T1.js', //for -- String.fromCharCode()
		//'15.10.2.12_A3_T2.js', //for -- String.fromCharCode()
		//'15.10.2.12_A3_T3.js', //for -- String.fromCharCode()
		//'15.10.2.12_A3_T4.js', //for -- String.fromCharCode()
		//'15.10.2.12_A3_T5.js', //while -- lastIndex ???
		//'15.10.2.12_A4_T1.js', //for -- String.fromCharCode()
		//'15.10.2.12_A4_T2.js', //for -- String.fromCharCode()
		//'15.10.2.12_A4_T3.js', //for -- String.fromCharCode()
		//'15.10.2.12_A4_T4.js', //for -- String.fromCharCode()
		//'15.10.2.12_A4_T5.js', //while -- lastIndex ???
		//'15.10.2.12_A5_T1.js', //for -- String.fromCharCode()
		//'15.10.2.12_A5_T2.js', //for -- String.fromCharCode()
		//'15.10.2.12_A5_T3.js', //for -- String.fromCharCode()
		//'15.10.2.12_A5_T4.js', //while -- lastIndex ???
		//'15.10.2.12_A6_T1.js', //for -- String.fromCharCode()
		//'15.10.2.12_A6_T2.js', //for -- String.fromCharCode()
		//'15.10.2.12_A6_T3.js', //for -- String.fromCharCode()
		//'15.10.2.12_A6_T4.js', //while -- lastIndex ???
		'15.10.2.13_A1_T1.js',
		'15.10.2.13_A1_T2.js',
		'15.10.2.13_A1_T3.js', //for
		'15.10.2.13_A1_T4.js', //for
		'15.10.2.13_A1_T5.js', //for
		'15.10.2.13_A1_T6.js', //for
		'15.10.2.13_A1_T7.js',
		'15.10.2.13_A1_T8.js', //for
		'15.10.2.13_A1_T9.js', //for
		//'15.10.2.13_A1_T10.js', //syntax error --- __executed = /[a-c\d]+/.exec("\n\n\abc324234\n");
		'15.10.2.13_A1_T11.js', //for
		'15.10.2.13_A1_T12.js', //for
		'15.10.2.13_A1_T13.js', //for
		'15.10.2.13_A1_T14.js', //for
		'15.10.2.13_A1_T15.js', //for
		//'15.10.2.13_A1_T16.js', //BAD TEST - RegExp error  --- /[\12-\14]/
		'15.10.2.13_A1_T17.js',
		'15.10.2.13_A2_T1.js', //for
		//'15.10.2.13_A2_T2.js', //syntax error
		'15.10.2.13_A2_T3.js', //for
		'15.10.2.13_A2_T4.js', //for
		'15.10.2.13_A2_T5.js', //for
		'15.10.2.13_A2_T6.js',
		'15.10.2.13_A2_T7.js', //for
		'15.10.2.13_A2_T8.js', //for
		'15.10.2.13_A3_T1.js', //for
		'15.10.2.13_A3_T2.js', //for
		'15.10.2.13_A3_T3.js', //for
		'15.10.2.13_A3_T4.js', //for
		//'15.10.2.15_A1_T1.js', //Assertion failed
		//'15.10.2.15_A1_T2.js', //Assertion failed
		//'15.10.2.15_A1_T3.js', //Assertion failed
		//'15.10.2.15_A1_T4.js', //Assertion failed
		//'15.10.2.15_A1_T5.js', //Assertion failed
		//'15.10.2.15_A1_T6.js', //Assertion failed
		//'15.10.2.15_A1_T7.js', //Assertion failed
		//'15.10.2.15_A1_T8.js', //Assertion failed
		//'15.10.2.15_A1_T9.js', //Assertion failed
		//'15.10.2.15_A1_T10.js', //Assertion failed
		//'15.10.2.15_A1_T11.js', //Assertion failed
		//'15.10.2.15_A1_T12.js', //Assertion failed
		//'15.10.2.15_A1_T13.js', //Assertion failed
		//'15.10.2.15_A1_T14.js', //Assertion failed
		//'15.10.2.15_A1_T15.js', //Assertion failed
		//'15.10.2.15_A1_T16.js', //Assertion failed
		//'15.10.2.15_A1_T17.js', //Assertion failed
		//'15.10.2.15_A1_T18.js', //Assertion failed
		//'15.10.2.15_A1_T19.js', //Assertion failed
		//'15.10.2.15_A1_T20.js', //Assertion failed
		//'15.10.2.15_A1_T21.js', //Assertion failed
		//'15.10.2.15_A1_T22.js', //Assertion failed
		//'15.10.2.15_A1_T23.js', //Assertion failed
		//'15.10.2.15_A1_T24.js', //Assertion failed
		//'15.10.2.15_A1_T25.js', //Assertion failed
		//'15.10.2.15_A1_T26.js', //Assertion failed
		//'15.10.2.15_A1_T27.js', //Assertion failed
		//'15.10.2.15_A1_T28.js', //Assertion failed
		//'15.10.2.15_A1_T29.js', //Assertion failed
		//'15.10.2.15_A1_T30.js', //Assertion failed
		//'15.10.2.15_A1_T31.js', //Assertion failed
		//'15.10.2.15_A1_T32.js', //Assertion failed
		//'15.10.2.15_A1_T33.js', //Assertion failed
		//'15.10.2.15_A1_T34.js', //Assertion failed
		//'15.10.2.15_A1_T35.js', //Assertion failed
		//'15.10.2.15_A1_T36.js', //Assertion failed
		//'15.10.2.15_A1_T37.js', //Assertion failed
		//'15.10.2.15_A1_T38.js', //Assertion failed
		//'15.10.2.15_A1_T39.js', //Assertion failed
		//'15.10.2.15_A1_T40.js', //Assertion failed
		//'15.10.2.15_A1_T41.js', //Assertion failed
		//'15.10.2.15-3-1.js', //catch
		//'15.10.2.15-3-2.js', //catch
		//'15.10.2.15-6-1.js', //catch
		//'15.10.2.2-1.js', //new
		'15.10.2.3_A1_T1.js', //for
		'15.10.2.3_A1_T2.js', //for
		'15.10.2.3_A1_T3.js', //for
		'15.10.2.3_A1_T4.js', //for
		'15.10.2.3_A1_T5.js',
		'15.10.2.3_A1_T6.js', //for
		'15.10.2.3_A1_T7.js',
		'15.10.2.3_A1_T8.js', //for
		'15.10.2.3_A1_T9.js', //for
		'15.10.2.3_A1_T10.js', //for
		'15.10.2.3_A1_T11.js', //for
		'15.10.2.3_A1_T12.js', //for
		'15.10.2.3_A1_T13.js', //for
		'15.10.2.3_A1_T14.js', //for
		'15.10.2.3_A1_T15.js', //for
		'15.10.2.3_A1_T16.js', //for
		'15.10.2.3_A1_T17.js', //for
		'15.10.2.5_A1_T1.js', //for
		'15.10.2.5_A1_T2.js', //for
		'15.10.2.5_A1_T3.js', //for
		'15.10.2.5_A1_T4.js', //for
		//'15.10.2.5_A1_T5.js', //RegExp error ---- /(a*)b\1+/.exec("baaaac")
		//'15.10.2.5-3-1.js', //catch
		'15.10.2.6_A1_T1.js',
		'15.10.2.6_A1_T2.js', //for
		'15.10.2.6_A1_T3.js', //for
		'15.10.2.6_A1_T4.js', //for
		'15.10.2.6_A1_T5.js', //for
		'15.10.2.6_A2_T1.js',
		'15.10.2.6_A2_T2.js', //for
		'15.10.2.6_A2_T3.js', //for
		'15.10.2.6_A2_T4.js', //for
		'15.10.2.6_A2_T5.js', //for
		'15.10.2.6_A2_T6.js', //for
		'15.10.2.6_A2_T7.js',
		'15.10.2.6_A2_T8.js',
		'15.10.2.6_A2_T9.js', //for
		'15.10.2.6_A2_T10.js', //for
		'15.10.2.6_A3_T1.js', //for
		'15.10.2.6_A3_T2.js', //for
		'15.10.2.6_A3_T3.js',
		'15.10.2.6_A3_T4.js', //for
		'15.10.2.6_A3_T5.js',
		'15.10.2.6_A3_T6.js', //for
		'15.10.2.6_A3_T7.js', //for
		'15.10.2.6_A3_T8.js', //for
		'15.10.2.6_A3_T9.js',
		'15.10.2.6_A3_T10.js', //for
		'15.10.2.6_A3_T11.js', //for
		'15.10.2.6_A3_T12.js', //for
		'15.10.2.6_A3_T13.js',
		'15.10.2.6_A3_T14.js', //for
		'15.10.2.6_A3_T15.js',
		'15.10.2.6_A4_T1.js', //for
		'15.10.2.6_A4_T2.js', //for
		'15.10.2.6_A4_T3.js', //for
		'15.10.2.6_A4_T4.js', //for
		'15.10.2.6_A4_T5.js', //for
		'15.10.2.6_A4_T6.js', //for
		'15.10.2.6_A4_T7.js',
		'15.10.2.6_A4_T8.js', //for
		'15.10.2.6_A5_T1.js', //for
		'15.10.2.6_A5_T2.js', //for
		'15.10.2.6_A6_T1.js', //for
		'15.10.2.6_A6_T2.js', //for
		'15.10.2.6_A6_T3.js', //for
		'15.10.2.6_A6_T4.js', //for
		'15.10.2.7_A1_T1.js', //for
		'15.10.2.7_A1_T2.js',
		'15.10.2.7_A1_T3.js', //for
		'15.10.2.7_A1_T4.js', //for
		'15.10.2.7_A1_T5.js', //for
		'15.10.2.7_A1_T6.js', //for
		'15.10.2.7_A1_T7.js', //for
		'15.10.2.7_A1_T8.js', //for
		'15.10.2.7_A1_T9.js',
		'15.10.2.7_A1_T10.js', //for
		'15.10.2.7_A1_T11.js', //for
		'15.10.2.7_A1_T12.js', //for
		'15.10.2.7_A2_T1.js', //for
		'15.10.2.7_A2_T2.js', //for
		'15.10.2.7_A2_T3.js', //for
		'15.10.2.7_A2_T4.js',
		//'15.10.2.7_A3_T1.js', //syntax error
		'15.10.2.7_A3_T2.js', //for
		'15.10.2.7_A3_T3.js',
		'15.10.2.7_A3_T4.js',
		'15.10.2.7_A3_T5.js', //for
		'15.10.2.7_A3_T6.js', //for
		'15.10.2.7_A3_T7.js', //for
		'15.10.2.7_A3_T8.js', //for
		'15.10.2.7_A3_T9.js', //for
		'15.10.2.7_A3_T10.js',
		'15.10.2.7_A3_T11.js', //for
		'15.10.2.7_A3_T12.js', //for
		'15.10.2.7_A3_T13.js', //for
		'15.10.2.7_A3_T14.js', //for
		'15.10.2.7_A4_T1.js', //for
		'15.10.2.7_A4_T2.js', //for
		'15.10.2.7_A4_T3.js', //for
		'15.10.2.7_A4_T4.js', //for
		'15.10.2.7_A4_T5.js', //for
		//'15.10.2.7_A4_T6.js', //syntax error
		//'15.10.2.7_A4_T7.js', //syntax error
		'15.10.2.7_A4_T8.js',
		'15.10.2.7_A4_T9.js', //for
		'15.10.2.7_A4_T10.js', //for
		'15.10.2.7_A4_T11.js', //for
		'15.10.2.7_A4_T12.js', //for
		'15.10.2.7_A4_T13.js', //for
		'15.10.2.7_A4_T14.js', //for
		'15.10.2.7_A4_T15.js', //for
		'15.10.2.7_A4_T16.js', //for
		'15.10.2.7_A4_T17.js', //for
		'15.10.2.7_A4_T18.js', //for
		'15.10.2.7_A4_T19.js', //for
		'15.10.2.7_A4_T20.js', //for
		'15.10.2.7_A4_T21.js',
		'15.10.2.7_A5_T1.js', //for
		'15.10.2.7_A5_T2.js', //for
		'15.10.2.7_A5_T3.js',
		'15.10.2.7_A5_T4.js', //for
		'15.10.2.7_A5_T5.js', //for
		'15.10.2.7_A5_T6.js', //for
		'15.10.2.7_A5_T7.js', //for
		'15.10.2.7_A5_T8.js', //for
		'15.10.2.7_A5_T9.js', //for
		'15.10.2.7_A5_T10.js', //for
		'15.10.2.7_A5_T11.js', //for
		'15.10.2.7_A5_T12.js', //for
		'15.10.2.7_A6_T1.js', //for
		'15.10.2.7_A6_T2.js',
		'15.10.2.7_A6_T3.js', //for
		'15.10.2.7_A6_T4.js', //for
		'15.10.2.7_A6_T5.js', //for
		'15.10.2.7_A6_T6.js', //for
		'15.10.2.8_A1_T1.js', //for
		'15.10.2.8_A1_T2.js', //for
		'15.10.2.8_A1_T3.js', //for
		'15.10.2.8_A1_T4.js', //for
		'15.10.2.8_A1_T5.js',
		'15.10.2.8_A2_T1.js', //for
		'15.10.2.8_A2_T2.js', //for
		'15.10.2.8_A2_T3.js',
		'15.10.2.8_A2_T4.js',
		'15.10.2.8_A2_T5.js', //for
		'15.10.2.8_A2_T6.js', //for
		'15.10.2.8_A2_T7.js', //for
		'15.10.2.8_A2_T8.js',
		'15.10.2.8_A2_T9.js', //for
		'15.10.2.8_A2_T10.js', //for
		'15.10.2.8_A2_T11.js', //for
		'15.10.2.8_A3_T1.js', //for
		'15.10.2.8_A3_T2.js', //for
		'15.10.2.8_A3_T3.js',
		'15.10.2.8_A3_T4.js', //for
		'15.10.2.8_A3_T5.js', //for
		'15.10.2.8_A3_T6.js', //for
		'15.10.2.8_A3_T7.js', //for
		'15.10.2.8_A3_T8.js', //for
		'15.10.2.8_A3_T9.js', //for
		'15.10.2.8_A3_T10.js', //for
		'15.10.2.8_A3_T11.js', //for
		'15.10.2.8_A3_T12.js', //for
		'15.10.2.8_A3_T13.js', //for
		'15.10.2.8_A3_T14.js', //for
		//'15.10.2.8_A3_T15.js', //syntax error
		//'15.10.2.8_A3_T16.js', //syntax error
		//'15.10.2.8_A3_T17.js', //syntax error
		'15.10.2.8_A3_T18.js',
		'15.10.2.8_A3_T19.js',
		'15.10.2.8_A3_T20.js', //for
		'15.10.2.8_A3_T21.js', //for
		'15.10.2.8_A3_T22.js', //for
		'15.10.2.8_A3_T23.js', //for
		'15.10.2.8_A3_T24.js', //for
		'15.10.2.8_A3_T25.js', //for
		'15.10.2.8_A3_T26.js', //for
		'15.10.2.8_A3_T27.js', //for
		'15.10.2.8_A3_T28.js', //for
		'15.10.2.8_A3_T29.js', //for
		'15.10.2.8_A3_T30.js', //for
		'15.10.2.8_A3_T31.js', //for
		'15.10.2.8_A3_T32.js', //for
		'15.10.2.8_A3_T33.js', //for
		'15.10.2.8_A4_T1.js', //for
		'15.10.2.8_A4_T2.js', //for
		'15.10.2.8_A4_T3.js', //for
		'15.10.2.8_A4_T4.js', //for
		'15.10.2.8_A4_T5.js', //for
		'15.10.2.8_A4_T6.js', //for
		'15.10.2.8_A4_T7.js', //for
		'15.10.2.8_A4_T8.js', //for
		'15.10.2.8_A4_T9.js', //for
		'15.10.2.8_A5_T1.js', //for
		'15.10.2.8_A5_T2.js', //for
		'15.10.2.9_A1_T1.js', //for
		'15.10.2.9_A1_T2.js', //for
		'15.10.2.9_A1_T3.js', //for
		//'15.10.2.9_A1_T4.js', //BAD TEST - RegExp error --- /\b(\w+) \2\b/
		//'15.10.2.9_A1_T5.js', //RegExp error --- /(a*)b\1+/
		//'15.10.2_A1_T1.js', //syntax error
		'15.10.3.1_A1_T1.js',
		'15.10.3.1_A1_T2.js',
		'15.10.3.1_A1_T3.js',
		//'15.10.3.1_A1_T4.js', //type error
		'15.10.3.1_A1_T5.js',
		//'15.10.3.1_A2_T1.js', //Assertion failed
		//'15.10.3.1_A2_T2.js', //catch
		'15.10.3.1_A3_T1.js',
		'15.10.3.1_A3_T2.js',
		//'15.10.4.1_A1_T1.js', //Assertion failed
		//'15.10.4.1_A1_T2.js', //Assertion failed
		//'15.10.4.1_A1_T3.js', //Assertion failed
		//'15.10.4.1_A1_T4.js', //Assertion failed
		//'15.10.4.1_A1_T5.js', //Assertion failed
		//'15.10.4.1_A2_T1.js', //Assertion failed
		//'15.10.4.1_A2_T2.js', //Assertion failed
		'15.10.4.1_A3_T1.js',
		//'15.10.4.1_A3_T2.js', //Assertion failed
		//'15.10.4.1_A3_T3.js', //Assertion failed
		//'15.10.4.1_A3_T4.js', //Assertion failed
		//'15.10.4.1_A3_T5.js', //Assertion failed
		//'15.10.4.1_A4_T1.js', //Assertion failed
		//'15.10.4.1_A4_T2.js', //Assertion failed
		//'15.10.4.1_A4_T3.js', //Assertion failed
		//'15.10.4.1_A4_T4.js', //Assertion failed
		//'15.10.4.1_A4_T5.js', //Assertion failed
		//'15.10.4.1_A5_T1.js', //Assertion failed
		//'15.10.4.1_A5_T2.js', //Assertion failed
		//'15.10.4.1_A5_T3.js', //try
		//'15.10.4.1_A5_T4.js', //Assertion failed
		//'15.10.4.1_A5_T6.js', //Assertion failed
		//'15.10.4.1_A5_T7.js', //Assertion failed
		//'15.10.4.1_A5_T8.js', //Assertion failed
		//'15.10.4.1_A5_T9.js', //Assertion failed
		//'15.10.4.1_A6_T1.js', //Object.prototype.toString
		//'15.10.4.1_A7_T1.js', //type error
		//'15.10.4.1_A7_T2.js', //RegExp.prototype.isPrototypeOf(__re)
		//'15.10.4.1_A8_T1.js', //Assertion failed
		//'15.10.4.1_A8_T2.js', //Assertion failed
		//'15.10.4.1_A8_T3.js', //Assertion failed
		//'15.10.4.1_A8_T4.js', //__re.ignoreCase
		//'15.10.4.1_A8_T5.js', //Assertion failed
		//'15.10.4.1_A8_T6.js', //try
		//'15.10.4.1_A8_T7.js', //try
		//'15.10.4.1_A8_T8.js', //try
		//'15.10.4.1_A8_T9.js', //Assertion failed
		//'15.10.4.1_A8_T10.js', //Assertion failed
		//'15.10.4.1_A8_T11.js', //new
		//'15.10.4.1_A8_T12.js', //try
		//'15.10.4.1_A8_T13.js', //try
		//'15.10.4.1_A9_T1.js', //Assertion failed
		//'15.10.4.1_A9_T2.js', //Assertion failed
		//'15.10.4.1_A9_T3.js', //Assertion failed
		//'15.10.4.1-1.js', //catch
		//'15.10.4.1-2.js', //new
		//'15.10.4.1-3.js', //catch
		//'15.10.4.1-4.js', //catch
		//'15.10.5.1_A1.js', //RegExp.hasOwnProperty('prototype')
		//'15.10.5.1_A2.js', //RegExp.hasOwnProperty('prototype')
		//'15.10.5.1_A3.js', //RegExp.hasOwnProperty('prototype')
		//'15.10.5.1_A4.js', //RegExp.hasOwnProperty('prototype')
		'15.10.5_A1.js',
		//'15.10.5_A2_T1.js', //Function.prototype.isPrototypeOf(RegExp)
		//'15.10.5_A2_T2.js', //type error
		'15.10.6.1_A1_T1.js',
		//'15.10.6.1_A1_T2.js', //syntax error
		//'15.10.6.2_A1_T1.js', //syntax error
		//'15.10.6.2_A1_T2.js', //with
		//'15.10.6.2_A1_T3.js', //Assertion failed
		//'15.10.6.2_A1_T4.js', //syntax error
		//'15.10.6.2_A1_T5.js', //??????????????????????
		//'15.10.6.2_A1_T6.js', //syntax error
		//'15.10.6.2_A1_T7.js', //try
		//'15.10.6.2_A1_T8.js', //try
		'15.10.6.2_A1_T9.js',
		//'15.10.6.2_A1_T10.js', //syntax error
		//'15.10.6.2_A1_T11.js', //Assertion failed
		//'15.10.6.2_A1_T12.js', //??????????????????????
		//'15.10.6.2_A1_T13.js', //syntax error
		//'15.10.6.2_A1_T14.js', //syntax error
		//'15.10.6.2_A1_T15.js', //??????????????????????
		'15.10.6.2_A1_T16.js',
		//'15.10.6.2_A1_T17.js', //syntax error
		//'15.10.6.2_A1_T18.js', //syntax error
		//'15.10.6.2_A1_T19.js', //syntax error
		//'15.10.6.2_A1_T20.js', //syntax error
		//'15.10.6.2_A1_T21.js', //syntax error
		//'15.10.6.2_A2_T1.js', //syntax error
		//'15.10.6.2_A2_T2.js', //try
		//'15.10.6.2_A2_T3.js', //try
		//'15.10.6.2_A2_T4.js', //Assertion failed
		//'15.10.6.2_A2_T5.js', //new Boolean()
		//'15.10.6.2_A2_T6.js', //Assertion failed
		//'15.10.6.2_A2_T7.js', //try
		//'15.10.6.2_A2_T8.js', //try
		//'15.10.6.2_A2_T9.js', //try
		//'15.10.6.2_A2_T10.js', //try
		//'15.10.6.2_A3_T1.js', //do
		//'15.10.6.2_A3_T2.js', //syntax error
		//'15.10.6.2_A3_T3.js', //syntax error
		//'15.10.6.2_A3_T4.js', //syntax error
		//'15.10.6.2_A3_T5.js', //do
		//'15.10.6.2_A3_T6.js', //do
		//'15.10.6.2_A3_T7.js', //do
		//'15.10.6.2_A4_T1.js', //syntax error
		//'15.10.6.2_A4_T2.js', //syntax error
		//'15.10.6.2_A4_T3.js', //syntax error
		//'15.10.6.2_A4_T4.js', //syntax error
		//'15.10.6.2_A4_T5.js', //syntax error
		//'15.10.6.2_A4_T6.js', //syntax error
		//'15.10.6.2_A4_T7.js', //syntax error
		//'15.10.6.2_A4_T8.js', //syntax error
		//'15.10.6.2_A4_T9.js', //syntax error
		//'15.10.6.2_A4_T10.js', //syntax error
		//'15.10.6.2_A4_T11.js', //syntax error
		//'15.10.6.2_A4_T12.js', //syntax error
		//'15.10.6.2_A5_T1.js', //syntax error
		'15.10.6.2_A5_T2.js',
		'15.10.6.2_A5_T3.js',
		'15.10.6.2_A6.js',
		//'15.10.6.2_A7.js', //catch
		//'15.10.6.2_A8.js', //RegExp.prototype.exec.hasOwnProperty('length')
		//'15.10.6.2_A9.js', //RegExp.prototype.exec.hasOwnProperty('length')
		//'15.10.6.2_A10.js', //RegExp.prototype.exec.hasOwnProperty('length')
		//'15.10.6.2_A11.js', //RegExp.prototype.exec.hasOwnProperty('length')
		//'15.10.6.2_A12.js', //Assertion failed
		//'15.10.6.2-9-e-1.js', //regObj.exec(str).toString()
		'15.10.6.3_A1_T1.js',
		//'15.10.6.3_A1_T2.js', //Assertion failed
		//'15.10.6.3_A1_T3.js', //Assertion failed
		'15.10.6.3_A1_T4.js',
		//'15.10.6.3_A1_T5.js', //???????????????????????
		'15.10.6.3_A1_T6.js',
		//'15.10.6.3_A1_T7.js', //try
		//'15.10.6.3_A1_T8.js', //try
		'15.10.6.3_A1_T9.js',
		'15.10.6.3_A1_T10.js',
		//'15.10.6.3_A1_T11.js', //Assertion failed
		//'15.10.6.3_A1_T12.js', //???????????????????????
		'15.10.6.3_A1_T13.js',
		'15.10.6.3_A1_T14.js',
		//'15.10.6.3_A1_T15.js', //???????????????????????
		'15.10.6.3_A1_T16.js',
		'15.10.6.3_A1_T17.js',
		'15.10.6.3_A1_T18.js',
		'15.10.6.3_A1_T19.js',
		'15.10.6.3_A1_T20.js',
		'15.10.6.3_A1_T21.js',
		//'15.10.6.3_A10.js', //RegExp.prototype.test.hasOwnProperty('length')
		//'15.10.6.3_A11.js', //RegExp.prototype.test.hasOwnProperty('length')
		//'15.10.6.3_A2_T1.js', //syntax error
		//'15.10.6.3_A2_T2.js', //try
		//'15.10.6.3_A2_T3.js', //try
		//'15.10.6.3_A2_T4.js', //Assertion failed
		//'15.10.6.3_A2_T5.js', //new Boolean()
		//'15.10.6.3_A2_T6.js', //Assertion failed
		//'15.10.6.3_A2_T7.js', //try
		//'15.10.6.3_A2_T8.js', //try
		//'15.10.6.3_A2_T9.js', //try
		//'15.10.6.3_A2_T10.js', //syntax error
		'15.10.6.3_A6.js',
		//'15.10.6.3_A7.js', //catch
		//'15.10.6.3_A8.js', //RegExp.prototype.test.hasOwnProperty('length')
		//'15.10.6.3_A9.js', //RegExp.prototype.test.hasOwnProperty('length')
		'15.10.6.4_A6.js',
		//'15.10.6.4_A7.js', //catch
		//'15.10.6.4_A8.js', //RegExp.prototype.toString.hasOwnProperty("length")
		//'15.10.6.4_A9.js', //RegExp.prototype.toString.hasOwnProperty("length")
		//'15.10.6.4_A10.js', //RegExp.prototype.toString.hasOwnProperty("length")
		//'15.10.6.4_A11.js', //RegExp.prototype.toString.hasOwnProperty("length")
		//'15.10.6.js', //Object.prototype.toString.call(RegExp.prototype)
		//'15.10.6_A1_T1.js', //Object.prototype.isPrototypeOf(RegExp.prototype)
		//'15.10.6_A1_T2.js', //RegExp.prototype.indicator
		//'15.10.7.1_A8.js', //Assertion failed
		//'15.10.7.1_A9.js', //__re.hasOwnProperty('source')
		//'15.10.7.1_A10.js', //__re.hasOwnProperty('source')
		'15.10.7.1-1.js',
		//'15.10.7.1-2.js', //Object.getOwnPropertyDescriptor(RegExp.prototype, 'source')
		//'15.10.7.2_A8.js', //Assertion failed
		//'15.10.7.2_A9.js', //__re.hasOwnProperty('global')
		//'15.10.7.2_A10.js', //__re.hasOwnProperty('global')
		'15.10.7.2-1.js',
		//'15.10.7.2-2.js', //Object.getOwnPropertyDescriptor(RegExp.prototype, 'global')
		//'15.10.7.3_A8.js', //Assertion failed
		//'15.10.7.3_A9.js', //__re.hasOwnProperty('ignoreCase')
		//'15.10.7.3_A10.js', //__re.hasOwnProperty('ignoreCase')
		'15.10.7.3-1.js',
		//'15.10.7.3-2.js', //Object.getOwnPropertyDescriptor(RegExp.prototype, 'ignoreCase')
		//'15.10.7.4_A8.js', //Assertion failed
		//'15.10.7.4_A9.js', //__re.hasOwnProperty('multiline')
		//'15.10.7.4_A10.js', //__re.hasOwnProperty('multiline')
		'15.10.7.4-1.js',
		//'15.10.7.4-2.js', //Object.getOwnPropertyDescriptor(RegExp.prototype, 'multiline')
		//'15.10.7.5_A8.js', //Assertion failed
		//'15.10.7.5_A9.js', //__re.hasOwnProperty('lastIndex')
		'15.10.7.5-1.js'//,
		//'15.10.7.5-2.js', //Object.getOwnPropertyDescriptor(RegExp.prototype, 'lastIndex')
		//'15.10.7_A1_T1.js', //syntax error
		//'15.10.7_A1_T2.js', //syntax error
		//'15.10.7_A2_T1.js', //new
		//'15.10.7_A2_T2.js', //new new
		//'15.10.7_A3_T1.js', //instanceof
		//'15.10.7_A3_T2.js' //instanceof
	];

	var header = "// Copyright (c) 2014 Cesanta Software\n// All rights reserved\n\nif (typeof console === \"object\") {\n\tprint = console.log;\n}\n\nvar $ERROR = function(msg) {\n\tprint(msg);\n\tnumFailedTests++;\n};\n\nfunction runTestCase(testcase) {\n\tif (testcase() !== true) {\n\t\t$ERROR('Test case returned non-true value!');\n\t}\n}\n/*\t\t\t\t\t\t*/\n\n";

	var fso = new ActiveXObject("Scripting.FileSystemObject");
	var testdir = 'test_suite\\';
	var testfilename = 'unit_test.js';
	var outtextstream = fso.CreateTextFile(testdir + testfilename, true);

	outtextstream.Write(header);


	function write_tests(tst_name, tst_folder, tst_array){
		var init = "var currFailed = 0, numTotalCases = 0, numFailedTests = 0, numExceptions = 0, numPassed = 0;\n\n";
		var footer = "/*\t\t\t\t\t\t*/\n\nprint('\\nTotal tests: ' + numTotalCases);\nprint('passed tests: ' + numPassed);\nprint('failed tests: ' + (numTotalCases-numPassed));\nprint('\\nfailed cases: ' + numFailedTests);\nprint('exceptions: ' + numExceptions);\n\n/*\t\t\t\t\t\t*/\n\n";
		outtextstream.Write("print('\\n" + tst_name + "');");
		outtextstream.Write(init);
		for(var i = 0; i < tst_array.length; i++){
			var intextstream = fso.OpenTextFile(tst_folder + tst_array[i], 1);
			outtextstream.Write('currFailed = numFailedTests + 1; currFailed--; numTotalCases++;\n');
			outtextstream.Write(intextstream.ReadAll())
			outtextstream.Write('if(currFailed == numFailedTests) numPassed++;\n\n');
			intextstream.Close();
		}

		outtextstream.Write(footer);
	}

	write_tests('ch.15.5', testdir + 'ch15\\15.5\\', files_15_5);

	write_tests('ch.15.10', testdir + 'ch15\\15.10\\', files_15_10);

	outtextstream.Close();

	(new ActiveXObject("WScript.Shell")).run("cmd /c \"@TITLE unit test ch. 15.5 and 15.10&.\\v7.exe " + testdir + testfilename + "&pause");
})();
