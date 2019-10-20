#include <stdio.h>
#include "javaClass.h"

int main(int argc, char *argv[]) {
    if (argc <= 1) {
        printf("Usage: ./class-exhibitor <class file>\n");
        return 1;
    }
    JavaClass jc;
    int status_file;
    status_file = openClassFile(&jc, argv[1]);
    printClassFileInfo(&jc);
    closeClassFile(&jc);
    return status_file;
}
