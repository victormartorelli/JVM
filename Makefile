all:
	gcc -std=c99 -Wall src/*.c -o class-exhibitor

.PHONY: examples
examples:
	for i in `ls examples/*.java`; do\
		javac -encoding utf8 -target 1.8 -source 1.8 $$i;\
	done;
	rm examples/*\$$*.class

.PHONY: javalang
javalang:
	javac -encoding utf8 -target 1.8 -source 1.8 java/lang/*.java

.PHONY: output
output:
	mkdir -p output
	for i in `ls examples/*.class`; do\
		./class-exhibitor $$i > output/$$(echo $$i | sed -e "s/[a-z]*\///g").out;\
	done;
