#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>


 enum Token {
    UNKNOWN,
    WORD, //ABC
    INTEGER, // 33.333
    FLOAT,
    STRING, // "xxx"
    COLON, //:
    COMMENT, //#
    OBJECT,
    BRACE_START, // {
    BRACE_END, //}
    EOF_,
} ;

class Value{
public:
    Value(){};
    Value(const std::string k, const std::string val, const Token typ):key(k),txtVal(val),type(typ){
        if(typ==INTEGER){
            intVal = atoi(val.c_str());
        }else if(typ==FLOAT){
            fltVal  = atof(val.c_str());
        }
    };
    Value(const std::string k, const std::vector<Value>&val, const Token typ):key(k),type(typ){
        objVal.assign(val.begin(),val.end());
    };
    Value(const Value&v){
        key=v.key;
        txtVal = v.txtVal;
        objVal.assign(v.objVal.begin(),v.objVal.end());
        type = v.type;
        intVal = v.intVal;
        fltVal = v.fltVal;

    }
    std::string String(){
        if(type!=OBJECT){
            if(type==INTEGER){
                char buf [64];
                sprintf(buf,"%s:%d",key.c_str(),intVal);
                return  std::string(buf);
            }
            if(type==FLOAT){
               char buf [64];
                sprintf(buf,"%s:%f",key.c_str(),fltVal);
                return  std::string(buf);
            }
            return key+":"+txtVal;
        }
        std::string txt = key+"{\n";
        for(Value& v: objVal){
            txt+= v.String()+"\n";
        }
        txt+="}";

        return txt;
    }
public:
    std::string key;
    std::string txtVal;
    int intVal;
    double fltVal;
    std::vector<Value> objVal;
    Token type;
};
inline int isSplitChar(char ch) {
    switch (ch) {
    case ':':
    case '{':
    case '}':
        return 1;
    }
    return 0;
}
inline int isWhiteChar(char ch) {
    switch (ch) {
    case ' ':
    case '\t':
    case '\n':
        return 1;
    }
    return 0;
}
//下一个非空字符
inline int nextChar(FILE* fp) {

    int ch = -1;
    do {
        ch = fgetc(fp);
    } while (isWhiteChar(ch));
    return ch;
}

inline int isInteger(const char* str, int size){
    if(size<1){
        return 0;
    }

    int i = 0;
    if(str[0]=='-'||str[0]=='+'){
        i =1;
    }
    for(;i<size;i++){
        if(str[i]<'0'||str[i]>'9'){
            return 0;
        }
    }
    return 1;
}
inline int isFloat(const char* str, int size){
      if(size<1){
        return 0;
    }
    int i = 0;
    if(str[0]=='-'||str[0]=='+'){
        i =1;
    }
    int dot = 0;
    for(;i<size;i++){
        if(str[i]<'0'||str[i]>'9'){
            if(!dot&&str[i]=='.'){
                dot =1;
            }else{
                return 0;
            }
        }
    }
    return 1;
}

static Token readToken(FILE* fp, char* buf) {
    int ch = nextChar(fp);
    int bi = 0;
    buf[bi++] = ch;
    if (ch == EOF) {
        buf[0] = 0;
        return EOF_;
    } else if (ch == ':') {
        buf[bi] = 0;
        return COLON;
    } else if (ch == '{') {
        buf[bi] = 0;
        return BRACE_START;
    } else if (ch == '}') {
        buf[bi] = 0;
        return BRACE_END;
    } else if (ch == '"') {
        // read until next " without \\ in pre
        bi = 0;
        int lastch = ch;
        while (1) {
            ch = fgetc(fp);
            if (ch == '"' && lastch != '\\') {
                buf[bi] = 0;
                return STRING;
            }
            buf[bi++] = ch;
            lastch = ch;
        }
    } else if (ch == '#') {
        //read until \n
        while (1) {
            ch = fgetc(fp);
            if (ch == '\n') {
                buf[bi] = 0;
                return COMMENT;
            }
            buf[bi++] = ch;
        }
    } else {
        //read until white or split char
        while (1) {
            ch = fgetc(fp);
            if (isWhiteChar(ch)) {
                buf[bi] = 0;
                if(isInteger(buf,bi)){
                    return INTEGER;
                }
                if(isFloat(buf,bi)){
                    return FLOAT;
                }    
                return WORD;
            }
            if (isSplitChar(ch)) {
                // 回退一个字符
                fseek(fp, -1, SEEK_CUR);
                buf[bi] = 0;
                if(isInteger(buf,bi)){
                    return INTEGER;
                }
                if(isFloat(buf,bi)){
                    return FLOAT;
                }                
                return WORD;
            }
            buf[bi++] = ch;
        }
    }
    return UNKNOWN;
}


static void parse(FILE* fp, std::vector<Value>& list) {
    while (1) {
        char buf[256];
        Token token = readToken(fp, buf);
        if (token == EOF_) {
            return;
        } else if (token == BRACE_END) {
            return;
        } else if (token == COMMENT) {
            list.push_back(Value("", buf,COMMENT));
            continue;
        }
        std::string key = buf;
        token = readToken(fp, buf);
        if (token == COLON) {
            token = readToken(fp, buf);
            if (token == BRACE_START) {
                //readObj
                std::vector<Value> objVal;
                parse(fp, objVal);
                Value v(key, objVal,OBJECT);
                list.push_back(v);
            } else {
                list.push_back(Value(key, buf,token));
            }
        } else if (token == BRACE_START) {
            //readObj
            std::vector<Value> objVal;
            parse(fp, objVal);
            list.push_back(Value(key, objVal,OBJECT));
        } else if (token == BRACE_END) {
            // endobj
            return;
        }
    }
}

Value prototxtParse(FILE* fp) {
    Value value;
    value.type = OBJECT;
    parse(fp, value.objVal);
    return value;
}



