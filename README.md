# prototxt and json parser

input text should be utf-8 format.

``` 
layer {
  name: "data"
  type: "Input"
  top: "data"
  label: "测试"
  input_param {
    shape {
      dim: 1.03
      dim: -5.3
      dim: 224
      dim: 224}
  }
}
#name: "ZF"

```

```C++
    FILE* fp = fopen(path, "rb");   
   
    pj_parser::Object obj = pj_parser::prototxtParse(fp);
    printf("%s\n",obj.String().c_str());
    for(pj_parser::Item& item :obj.list){
      //
    }
    obj.release();
```