var fs = require("fs");
var path = require("path");
var root = process.argv[2];
var files = fs.readdirSync(root);

var file_list = [];
var folder_list = [];

files.forEach(function(file){
if (file[0]!='.') {
	if (path.extname(file) == ".c") {
    file_list.push(path.basename(file, ".c") + ".o");  
  } else {
		if (fs.statSync(path.join(root,file)).isDirectory())
		{folder_list.push(file + "/");}
	}
}
})

var data = "";
file_list.forEach(function(item){
data = data + "obj-y += "+item+"\n";
})

folder_list.forEach(function(item){
data = data + "obj-y += "+item+"\n";
})

target = path.join(root, "Kbuild.mk");
fs.writeFileSync(target, data);

console.log(data)