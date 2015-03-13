#!/bin/sh

# generates compile_commands.json

echo '['
for file in src/*.c; do
    cat <<EOF
   {
        "directory": "/cesanta/v7/",
        "command": "cc -std=c99 ${file} -W -Wall -Wno-comment -Wno-variadic-macros -DV7_EXPOSE_PRIVATE",
        "file": "${file}"
    },
EOF
done
cat <<EOF
   {
        "directory": "dummy",
        "command": "please forgive my laziness",
        "file": "dummy.c"
   }
EOF
echo ']'
