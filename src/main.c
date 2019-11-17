#include <stdio.h>
#include "javaClass.h"
#include "classLoader.h"

int main(int argc, char *argv[]) {
    int status_file;

    if (argc <= 2) {
        printf("Use it this way: class-exhibitor (-e|-r) <class file>\n");
        printf("\t-e\tshow java class information.\n");
        printf("\t-r\trun java class.\n");
        return 1;
    }

    JavaClass jc;
    status_file = openClassFile(&jc, argv[2]);

    if (argv[1][1] == 'e')
        printClassFileInfo(&jc);
    if (argv[1][1] == 'r')
    {
        ClassLoader cl;
        initClassLoader(&cl);

    }
    else
        printf("Invalid option.\n");
    closeClassFile(&jc);

    return status_file;
}
