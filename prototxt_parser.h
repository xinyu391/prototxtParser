#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>

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


 enum Token {
    UNKNOWN,
    WORD, //ABC
    NUMBER, // 33.333
    STRING, // "xxx"
    COLON, //:
    COMMENT, //#
    OBJECT,
    BRACE_START, // {
    BRACE_END, //}
    EOF_,
} ;


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
                return WORD;
            }
            if (isSplitChar(ch)) {
                // 回退一个字符
                fseek(fp, -1, SEEK_CUR);
                buf[bi] = 0;
                return WORD;
            }
            buf[bi++] = ch;
        }
    }
    return UNKNOWN;
}

class Value{
public:
    Value(){};
    Value(const std::string k, const std::string val, const Token typ):key(k),txtVal(val),type(typ){};
    Value(const std::string k, const std::vector<Value>&val, const Token typ):key(k),type(typ){
        objVal.assign(val.begin(),val.end());
    };
    Value(const Value&v){
        key=v.key;
        txtVal = v.txtVal;
        objVal.assign(v.objVal.begin(),v.objVal.end());
        type = v.type;
    }
    std::string String(){
        if(type!=OBJECT){
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
    std::vector<Value> objVal;
    Token type;
};

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
                list.push_back(Value(key, buf,STRING));
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



