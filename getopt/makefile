
main: version.h main.c
	gcc main.c -o main

# 生成version.h文件
version.h:
	sh ./genversion.sh

.PHONY: clean

clean:
	@rm -fr main version.h