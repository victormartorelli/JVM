#include <stdio.h>
#include "javaClass.h"

int main(int argc, char *argv[]) {
    JavaClass jc;
    int status_file;
    
    if (argc <= 1) {
        printf("Use it this way: ./class-exhibitor <class file>\n");
        return 1;
    }

    status_file = openClassFile(&jc, argv[1]);
    printClassFileInfo(&jc);
    closeClassFile(&jc);
    
    return status_file;
}
