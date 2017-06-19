#!/bin/sh
#find . -name "*.h" -o -name "*.c" -o -name "*.cc" > cscope.files
find . -type f -print | grep -E '\.(c(pp)?|h(pp))$' > cscope.files
cscope -bkq -i cscope.files
#ctags -R
ctags -R --c++-kinds=+p --fields=+iaS --extra=+q
