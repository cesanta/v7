(function(){
	var files =		[
								'v7.h', 'license.h', 'utf.h', 'tokenizer.h', 'mbuf.h', 'ast.h',
								'parser.h', 'mm.h', 'internal.h', 'vm.h', 'gc.h', 'slre.h',
								'varint.h',
								'mbuf.c', 'utf.c', 'varint.c', 'tokenizer.c', 'array.c',
								'boolean.c', 'math.c', 'string.c', 'ast.c', 'vm.c', 'gc.c', 'parser.c',
								'interpreter.c', 'slre.c', 'object.c', 'error.c', 'number.c',
								'json.c', 'main.c', 'date.c', 'function.c', 'stdlib.c', 'regex.c'
								];

	var ch_pattern = /\.\.\/v7\.h/g;
  var ch_rep = 'v7.h';
	var ex_pattern = /#include "(v7|license|utf|tokenizer|mbuf|ast|parser|internal|vm|gc|slre|varint|mm)\.h"\r?\n/g;

	var fso = new ActiveXObject("Scripting.FileSystemObject");
	var outtextstream = fso.CreateTextFile("v7.c", true);

	for(var i = 0; i < files.length; i++){
		var intextstream = fso.OpenTextFile((i ? 'src\\': '') + files[i], 1);
		var fstr = intextstream.ReadAll();
		intextstream.Close();
		outtextstream.Write(fstr.replace(ch_pattern, ch_rep).replace(ex_pattern, ''));
	}

	outtextstream.Close();
})();
