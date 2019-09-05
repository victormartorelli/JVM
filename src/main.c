#include <stdio.h>

int main(int argc, char *argv[]) {
    if (argc <= 1) {
        printf("Usage: ./class-exhibitor [flags] <class file>\n"
               "where flags include:\n"
               "\t-v\t\textra verbosity\n");
        return 0;
    }
    return 0;
}
