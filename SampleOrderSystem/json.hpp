#pragma once
#include <string>
#include <vector>
#include <map>
#include <variant>
#include <stdexcept>
#include <sstream>
#include <fstream>
#include <functional>

class JsonValue {
public:
    using Null   = std::monostate;
    using Bool   = bool;
    using Number = double;
    using String = std::string;
    using Array  = std::vector<JsonValue>;
    using Object = std::map<std::string, JsonValue>;

    JsonValue()               : v_(Null{}) {}
    JsonValue(std::nullptr_t) : v_(Null{}) {}
    JsonValue(bool b)         : v_(Bool(b)) {}
    JsonValue(int n)          : v_(Number(n)) {}
    JsonValue(int64_t n)      : v_(Number(static_cast<double>(n))) {}
    JsonValue(double n)       : v_(Number(n)) {}
    JsonValue(const char* s)  : v_(String(s)) {}
    JsonValue(std::string s)  : v_(std::move(s)) {}
    JsonValue(Array a)        : v_(std::move(a)) {}
    JsonValue(Object o)       : v_(std::move(o)) {}

    bool isNull()   const { return std::holds_alternative<Null>(v_); }
    bool isBool()   const { return std::holds_alternative<Bool>(v_); }
    bool isNumber() const { return std::holds_alternative<Number>(v_); }
    bool isString() const { return std::holds_alternative<String>(v_); }
    bool isArray()  const { return std::holds_alternative<Array>(v_); }
    bool isObject() const { return std::holds_alternative<Object>(v_); }

    bool          getBool()   const { return std::get<Bool>(v_); }
    double        getNumber() const { return std::get<Number>(v_); }
    int64_t       getInt()    const { return static_cast<int64_t>(std::get<Number>(v_)); }
    const String& getString() const { return std::get<String>(v_); }
    const Array&  getArray()  const { return std::get<Array>(v_); }
    Array&        getArray()        { return std::get<Array>(v_); }
    const Object& getObject() const { return std::get<Object>(v_); }
    Object&       getObject()       { return std::get<Object>(v_); }

    JsonValue& operator[](const std::string& key) {
        if (isNull()) v_ = Object{};
        return std::get<Object>(v_)[key];
    }
    const JsonValue& at(const std::string& key) const {
        return std::get<Object>(v_).at(key);
    }
    bool contains(const std::string& key) const {
        if (!isObject()) return false;
        return std::get<Object>(v_).count(key) > 0;
    }
    JsonValue&       operator[](size_t idx)       { return std::get<Array>(v_)[idx]; }
    const JsonValue& operator[](size_t idx) const { return std::get<Array>(v_)[idx]; }
    size_t size() const {
        if (isArray())  return std::get<Array>(v_).size();
        if (isObject()) return std::get<Object>(v_).size();
        return 0;
    }
    void push_back(const JsonValue& val) {
        if (isNull()) v_ = Array{};
        std::get<Array>(v_).push_back(val);
    }
    static JsonValue parse(const std::string& text) { size_t i = 0; return parseValue(text, i); }
    static JsonValue parseFile(const std::string& path) {
        try {
            std::ifstream file(path);
            if (!file.is_open()) return JsonValue{};
            std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
            if (content.empty()) return JsonValue{};
            // UTF-8 BOM(\xEF\xBB\xBF) 제거 (Windows에서 파일 작성 시 추가될 수 있음)
            if (content.size() >= 3 &&
                (unsigned char)content[0] == 0xEF &&
                (unsigned char)content[1] == 0xBB &&
                (unsigned char)content[2] == 0xBF) {
                content = content.substr(3);
            }
            if (content.empty()) return JsonValue{};
            return parse(content);
        } catch (...) {
            return JsonValue{};  // 파싱 실패 시 null 반환
        }
    }
    std::string stringify(int indent = -1) const { std::ostringstream ss; toStream(ss, indent, 0); return ss.str(); }
    bool saveToFile(const std::string& path, int indent = 2) const {
        std::ofstream file(path);
        if (!file.is_open()) return false;
        file << stringify(indent);
        return true;
    }
private:
    std::variant<Null, Bool, Number, String, Array, Object> v_;
    static void skipWS(const std::string& s, size_t& i) { while (i < s.size() && std::isspace((unsigned char)s[i])) ++i; }
    static std::string parseString(const std::string& s, size_t& i) {
        ++i; std::string r;
        while (i < s.size() && s[i] != '"') {
            if (s[i] == '\\') { ++i; switch(s[i]){case '"':r+='"';break;case '\\':r+='\\';break;case 'n':r+='\n';break;case 'r':r+='\r';break;case 't':r+='\t';break;default:r+=s[i];break;} }
            else r += s[i];
            ++i;
        }
        ++i; return r;
    }
    static double parseNumber(const std::string& s, size_t& i) {
        size_t st=i; if(s[i]=='-')++i;
        while(i<s.size()&&std::isdigit((unsigned char)s[i]))++i;
        if(i<s.size()&&s[i]=='.'){ ++i; while(i<s.size()&&std::isdigit((unsigned char)s[i]))++i; }
        if(i<s.size()&&(s[i]=='e'||s[i]=='E')){ ++i; if(i<s.size()&&(s[i]=='+'||s[i]=='-'))++i; while(i<s.size()&&std::isdigit((unsigned char)s[i]))++i; }
        return std::stod(s.substr(st,i-st));
    }
    static Array parseArray(const std::string& s, size_t& i) {
        ++i; Array a; skipWS(s,i);
        if(i<s.size()&&s[i]==']'){++i;return a;}
        while(i<s.size()){a.push_back(parseValue(s,i));skipWS(s,i);if(i>=s.size())break;if(s[i]==']'){++i;break;}if(s[i]==','){++i;skipWS(s,i);}}
        return a;
    }
    static Object parseObject(const std::string& s, size_t& i) {
        ++i; Object o; skipWS(s,i);
        if(i<s.size()&&s[i]=='}'){++i;return o;}
        while(i<s.size()){skipWS(s,i);auto k=parseString(s,i);skipWS(s,i);++i;skipWS(s,i);o[k]=parseValue(s,i);skipWS(s,i);if(i>=s.size())break;if(s[i]=='}'){++i;break;}if(s[i]==',')++i;}
        return o;
    }
    static JsonValue parseValue(const std::string& s, size_t& i) {
        skipWS(s,i); if(i>=s.size()) throw std::runtime_error("EOF");
        char c=s[i];
        if(c=='"') return parseString(s,i);
        if(c=='{') return parseObject(s,i);
        if(c=='[') return parseArray(s,i);
        if(c=='t'&&s.compare(i,4,"true")==0){i+=4;return true;}
        if(c=='f'&&s.compare(i,5,"false")==0){i+=5;return false;}
        if(c=='n'&&s.compare(i,4,"null")==0){i+=4;return JsonValue{};}
        if(c=='-'||std::isdigit((unsigned char)c)) return parseNumber(s,i);
        throw std::runtime_error(std::string("Bad char: ")+c);
    }
    void toStream(std::ostringstream& ss, int indent, int depth) const {
        auto nl=[&](int d){ if(indent>=0){ss<<'\n';ss<<std::string((size_t)(indent*d),' ');} };
        if(isNull()){ss<<"null";return;}
        if(isBool()){ss<<(getBool()?"true":"false");return;}
        if(isNumber()){double n=getNumber();if(n==(double)(int64_t)n)ss<<(int64_t)n;else ss<<n;return;}
        if(isString()){ss<<'"';for(char c:getString()){switch(c){case '"':ss<<"\\\"";break;case '\\':ss<<"\\\\";break;case '\n':ss<<"\\n";break;case '\r':ss<<"\\r";break;case '\t':ss<<"\\t";break;default:ss<<c;}}ss<<'"';return;}
        if(isArray()){const auto&a=getArray();if(a.empty()){ss<<"[]";return;}ss<<'[';for(size_t k=0;k<a.size();++k){nl(depth+1);a[k].toStream(ss,indent,depth+1);if(k+1<a.size())ss<<',';}nl(depth);ss<<']';return;}
        if(isObject()){const auto&o=getObject();if(o.empty()){ss<<"{}";return;}ss<<'{';size_t k=0;for(const auto&[key,val]:o){nl(depth+1);ss<<'"'<<key<<'"'<<':';if(indent>=0)ss<<' ';val.toStream(ss,indent,depth+1);if(++k<o.size())ss<<',';}nl(depth);ss<<'}';}
    }
};
