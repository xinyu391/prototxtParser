#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <vector>
namespace pj_parser{


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
    ARRAY,
    ARRAY_START, // [
    ARRAY_END, // ]
    EOF_,
};
class Value;

class Item {
public:
    Item(std::string _key, Value* val) {
        key = _key;
        value = val;
    };
    void release();
    std::string key;
    Value* value;
};
class Object {
public:
    std::vector<Item> list;
    std::string String();
    void release();
};

class Value {
public:
    // std::string key;
    std::string txtVal; //string
    int intVal; // integer 
    double fltVal; // float 
    // bool bolVal;
    // null
    Object object;
    std::vector<Value*> array;
    Token type;

    Value(){};
    Value(const std::string val, const Token typ)
        : txtVal(val)
        , type(typ) {
        if (typ == INTEGER) {
            intVal = atoi(val.c_str());
        } else if (typ == FLOAT) {
            fltVal = atof(val.c_str());
        }
    };
    // array
    Value(std::vector<Value*>& val, const Token typ)
        : type(typ) {
        array.assign(val.begin(), val.end());
    };
    // object
    Value(const Object _obj, const Token typ)
        : type(typ) {
        // TODO
        object.list.assign(_obj.list.begin(), _obj.list.end());
    };
    Value(Value& v) {
        txtVal = v.txtVal;
        array.assign(v.array.begin(), v.array.end());
        object.list.assign(v.object.list.begin(), v.object.list.end());
        type = v.type;
        intVal = v.intVal;
        fltVal = v.fltVal;
    }

    Value operator=(Value& v) {
        txtVal = v.txtVal;
        intVal = v.intVal;
        fltVal = v.fltVal;

        array.assign(v.array.begin(), v.array.end());
        object.list.assign(v.object.list.begin(), v.object.list.end());
        return *this;
    }
    void release() {
        for (Value* val : array) {
            val->release();
            delete val;
        }
        object.release();
    }
    std::string String() {
        if (type == INTEGER) {
            char buf[64];
            sprintf(buf, "%d", intVal);
            return std::string(buf);
        }
        if (type == FLOAT) {
            char buf[64];
            sprintf(buf, "%f", fltVal);
            return std::string(buf);
        }
        if (type == WORD) {
            return txtVal;
        }
        if (type == OBJECT) {
            return object.String();
        }
        if (type == ARRAY) {
            std::string str = "[";
            for (int i=0;i<array.size();i++) {
                Value* val  = array[i];
                str += val->String() ;
                if(i<array.size()-1){
                    str +=",";
                }
            }
            str += "]\n";
            return str;
        }

        return "\"" + txtVal + "\"";
    }
};
inline void Item::release() {
    if (value != NULL) {
        value->release();
        delete value;
        value = NULL;
    }
}
inline std::string Object::String() {
    std::string str = "{\n";
    for (int i = 0; i < list.size(); i++) {
        if(list[i].value->type==COMMENT){
            str += list[i].value->String() + ",\n";
        }else{
            str += "\"" + list[i].key + "\":" + list[i].value->String() + ",\n";
        }
        
    }
    str += "}";
    return str;
}

inline void Object::release() {
    for (Item& item : list) {
        item.release();
    }
}


class Stream{
FILE* fp;
const char* ptr;
long ptr_size;
long offset;

public:
    Stream(FILE* _fp){
        fp  = _fp;
        ptr=0;
    }
    Stream(const char* _ptr, long _size){
        ptr = _ptr;
        ptr_size = _size;
        offset= 0;
        fp=0;
    }

    int getc(){
        if(fp){
            return fgetc(fp);
        }else{
            int c = ptr[offset++];
            return c;
        }
    }
    void seek(int off, int whence){
        if(fp){
             fseek(fp, off, SEEK_CUR);
        }else{
            switch (whence) {
            case SEEK_CUR:
                offset += off;
                break;
            case SEEK_END:
                offset = ptr_size + off;
                break;
            case SEEK_SET:
                offset = off;
                break;
            }
        }
    }
};
inline int isSplitChar(char ch) {
    switch (ch) {
    case ':':
    case '{':
    case '}':
    case '[':
    case ']':
    case ',':
        return 1;
    }
    return 0;
}
inline int isWhiteChar(char ch) {
    switch (ch) {
    case ' ':
    case ',':
    case '\t':
    case '\n':
        return 1;
    }
    return 0;
}
//下一个非空字符
inline int nextChar(Stream& stream) {

    int ch = -1;
    do {
        ch = stream.getc();
    } while (isWhiteChar(ch));
    return ch;
}

inline int isInteger(const char* str, int size) {
    if (size < 1) {
        return 0;
    }

    int i = 0;
    if (str[0] == '-' || str[0] == '+') {
        i = 1;
    }
    for (; i < size; i++) {
        if (str[i] < '0' || str[i] > '9') {
            return 0;
        }
    }
    return 1;
}
inline int isFloat(const char* str, int size) {
    if (size < 1) {
        return 0;
    }
    int i = 0;
    if (str[0] == '-' || str[0] == '+') {
        i = 1;
    }
    int dot = 0;
    for (; i < size; i++) {
        if (str[i] < '0' || str[i] > '9') {
            if (!dot && str[i] == '.') {
                dot = 1;
            } else {
                return 0;
            }
        }
    }
    return 1;
}

static Token readToken(Stream& stream, char* buf) {
    int ch = nextChar(stream);
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
    } else if (ch == '[') {
        buf[bi] = 0;
        return ARRAY_START;
    } else if (ch == ']') {
        buf[bi] = 0;
        return ARRAY_END;
    } else if (ch == '"') {
        // read until next " without \\ in pre
        bi = 0;
        int lastch = ch;
        while (1) {
            ch = stream.getc();
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
            ch = stream.getc();
            if (ch == '\n') {
                buf[bi] = 0;
                return COMMENT;
            }
            buf[bi++] = ch;
        }
    } else {
        //read until white or split char
        while (1) {
            ch = stream.getc();
            if (isWhiteChar(ch)) {
                buf[bi] = 0;
                if (isInteger(buf, bi)) {
                    return INTEGER;
                }
                if (isFloat(buf, bi)) {
                    return FLOAT;
                }
                return WORD;
            }
            if (isSplitChar(ch)) {
                // 回退一个字符
                stream.seek(-1, SEEK_CUR);
                buf[bi] = 0;
                if (isInteger(buf, bi)) {
                    return INTEGER;
                }
                if (isFloat(buf, bi)) {
                    return FLOAT;
                }
                return WORD;
            }
            buf[bi++] = ch;
        }
    }
    return UNKNOWN;
}
static void parseObj(Stream& stream, Object& value) ;

static void parseArray(Stream& stream, std::vector<Value*>& array) {
    char buf[256];
    for (;;) {
        Token tk = readToken(stream, buf);
        if (tk == ARRAY_END) {
            break;
        }
        if (tk == BRACE_START) {
            Object obj;
            parseObj(stream, obj);
            array.push_back(new Value(obj, OBJECT));
        } else if (tk == ARRAY_START) {
                Value* value = new Value();
                value->type = ARRAY;
                parseArray(stream, value->array);
                array.push_back(value);
        } else {
            array.push_back(new Value(buf, tk));
        }
    }
}
static void parseObj(Stream& stream, Object& value) {
    std::vector<Item>& list = value.list;
    int flag = 1;
    while (1) {
        char buf[256];
        Token token = readToken(stream, buf);
        if (flag && token == BRACE_START) {
            flag = 1;
            continue;
        }
        if (token == EOF_) {
            return;
        } else if (token == BRACE_END) {
            return;
        } else if (token == COMMENT) {
            // list.push_back(Item("", new Value(buf, COMMENT)));
            continue;
        }
        /*else if(token == BRACE_START){
            std::vector<Value> objVal;
            parse(fp, objVal);
            list.push_back(Value("", objVal,OBJECT));
        }*/

        std::string key = buf;
        token = readToken(stream, buf);
        if (token == COLON) {
            token = readToken(stream, buf);

            if (token == BRACE_START) { // object-value
                //begin object
                Object obj;
                parseObj(stream, obj);
                list.push_back(Item(std::string(key), new Value(obj, OBJECT)));
            } else if (token == ARRAY_START) { //array-value
                Value* value = new Value();
                value->type = ARRAY;
                parseArray(stream, value->array);
                list.push_back(Item(std::string(key), value));
            } else { // single-value
                list.push_back(Item(std::string(key), new Value(buf, token)));
            }
        } else if (token == BRACE_START) {
            //readObj
            Object obj;
            parseObj(stream, obj);
            list.push_back(Item(std::string(key), new Value(obj, OBJECT)));
        } else if (token == ARRAY_START) {
            // array, read value list

        } else if (token == ARRAY_END) {
            // array, read value list
            break;
        } else if (token == BRACE_END) {
            // endobj
            return;
        }
    }
}


inline Object prototxtParse(FILE* fp) {
    Object value;
    // Item.type = OBJECT;
    Stream stream(fp);
    parseObj(stream, value);
    return value;
}


inline Object prototxtParse(const char* str) {
    Object value;
    // Item.type = OBJECT;
    Stream stream(str, strlen(str));
    parseObj(stream, value);
    return value;
}

}