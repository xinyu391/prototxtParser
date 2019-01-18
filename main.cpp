#include <stdio.h>
#include <stdlib.h>
#include "prototxt_parser.h"

int main(int argc, char* argv[]) {
    char* path = "../sample.prototxt";
    if (argc > 1) {
        path = argv[1];
    }
    FILE* fp = fopen(path, "rb");
    
    Value val=  prototxtParse(fp);
    printf("size %d\n",val.objVal.size());
    printf("%s\n",val.String().c_str());

    fclose(fp);
    return 0;
}
