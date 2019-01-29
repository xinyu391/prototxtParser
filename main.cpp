#include <stdio.h>
#include <stdlib.h>
#include "prototxt_parser.h"
using namespace pj_parser;

int main(int argc, char* argv[]) {
    char* path = "../sample.prototxt";
    if (argc > 1) {
        path = argv[1];
    }
    FILE* fp = fopen(path, "rb");
    
    Object val=  prototxtParse(fp);
    printf("size %d\n",val.list.size());
    printf("%s\n",val.String().c_str());

val.release();
    fclose(fp);
    return 0;
}
