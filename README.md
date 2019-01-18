# prototxt parser
```C++
    FILE* fp = fopen(path, "rb");
   
    Value val=  prototxtParse(fp);
    printf("size %d\n",val.objVal.size());
    printf("%s\n",val.String().c_str());
```