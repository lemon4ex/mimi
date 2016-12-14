//
//  MimiParser.cpp
//  mimi
//
//  Created by lemon4ex on 16/4/27.
//  Copyright © 2016年 lemon4ex. All rights reserved.
//

#include "MimiParser.hpp"
#include <iostream>
#import <string>
#import <fcntl.h>
#import <dirent.h>
#import <unistd.h>
#import <sys/stat.h>

using namespace std;

const char *kConfusionOnceTag = "/*?--CONFUSED_ONCE--?*/"; ///< 只将此标记之后的一个标识符混淆
const char *kConfusionStartTag = "/*?--CONFUSED_START--?*/"; ///< 混淆块标记开始
const char *kConfusionEndTag = "/*?--CONFUSED_END--?*/"; ///< 混淆标记块结束

static size_t kConfusionOnceTagLength = strlen(kConfusionOnceTag);
static size_t kConfusionStartTagLength = strlen(kConfusionStartTag);
static size_t kConfusionEndTagLength = strlen(kConfusionEndTag);

bool MimiParser::Parse(const char *arg)
{
    struct stat st;
    stat(arg, &st);
    if (S_ISDIR(st.st_mode))
    {
        return ParseDir(arg);
    }
    else
    {
        return ParseFile(arg);
    }
}

bool MimiParser::ParseDir(const char *dir)
{
    DIR *dp = opendir(dir);
    if (dp)
    {
        char path[2048];
        strcpy(path, dir);
        char *subpath = path + strlen(path);
        *subpath++ = '/';
        
        //printf("PARSE Dir: %s\n", dir);
        struct dirent *ent;
        while ((ent = readdir(dp)))
        {
            if (ent->d_name[0] != '.')
            {
                strcpy(subpath, ent->d_name);
                char *endpath = subpath + strlen(subpath);
                if (ent->d_type == DT_DIR)
                {
                    ParseDir(path);
                }
                else if (!memcmp(endpath - 2, ".h", 2) || !memcmp(endpath - 2, ".m", 2) || !memcmp(endpath - 3, ".mm", 3))
                {
                    ParseFile(path);
                }
            }
        }
        closedir(dp);
        
        return true;
    }
    else
    {
        printf("无法打开文件夹： %s\n", dir);
        
        return false;
    }
}

bool MimiParser::ParseFile(const char *file)
{
    int fd = open(file, O_RDONLY);
    if (fd != -1)
    {
        printf("开始解析文件： %s\n", file);
        struct stat stat;
        fstat(fd, &stat);
        
        char *code = (char *)new char[stat.st_size + 1]();
        if (code)
        {
            read(fd, code, stat.st_size);
            
            ParseCode(code);
            
            delete [] code;
        }
        else
        {
            printf("无法申请足够内存，%s\n", file);
        }
        close(fd);
        
        return true;
    }
    else
    {
        printf("无法打开文件，%s\n", file);
        
        return false;
    }
}

bool MimiParser::ParseCode(const char *code)
{
    const char *p = code;
    while (*p)
    {
        if (const char *q = ParseCommon(p))
        {
            p = q;
        }
        else if (*p == '{')
        {
            p = ParseBlock(p);
        }
        else if (!memcmp(p, "@interface", sizeof("@interface") - 1))
        {
            p = ParseObject(p, MimiSymbolInterface);
        }
        else if (!memcmp(p, "@protocol", sizeof("@protocol") - 1))
        {
            p = ParseObject(p, MimiSymbolProtocol);
        }
        else if (!memcmp(p, "@implementation", sizeof("@implementation") - 1))
        {
            p = ParseObject(p, MimiSymbolImplementation);
        }
        else
        {
            p++;
        }
    }
    
    return true;
}

const char *MimiParser::ParseCommon(const char *code)
{
    if (*code == '#')
        return ParsePreprocessor(code);
    else if (*code == '\'' || *code == '\"')
        return ParseString(code);
    else if (*code == '/' && code[1] == '/')
        return ParseComment(code);
    else if (!memcmp(code, kConfusionEndTag, kConfusionEndTagLength))
    {
        if (m_confusionFlag != ConfusionAllInBlock) {
            printf("未匹配的混淆结束符号：%s\n", code);
        }
        m_confusionFlag = ConfusionIgnore;
        return code + kConfusionEndTagLength;
    }
    else if (*code == '/' && code[1] == '*')
    {
        if (!memcmp(code, kConfusionOnceTag, kConfusionOnceTagLength)) {
            m_confusionFlag = ConfusionOnce;
            return code + kConfusionOnceTagLength;
        }
        else if (!memcmp(code, kConfusionStartTag, kConfusionStartTagLength))
        {
            m_confusionFlag = ConfusionAllInBlock;
            return code + kConfusionStartTagLength;
        }
        else
        {
            return ParseComments(code);
        }
    }
    else if (*code == '\\')
        return code + 2;
    return NULL;
}

//const char *MimiParser::ParseConfusionOnce(const char *code)
//{
//    
//}
//
//const char *MimiParser::ParseConfusionBlock(const char *code)
//{
//    const char *p = code + sizeof(CONFUSION_BLOCK_START_FLAG) - 1;
//    
//    while (*p) {
//        if (!memcmp(p, CONFUSION_BLOCK_END_FLAG, sizeof(CONFUSION_BLOCK_END_FLAG) - 1))
//        {
//            if (m_confusionFlag != ConfusionAllInBlock) {
//                printf("未匹配的混淆结束符号：%s\n", p);
//            }
//            m_confusionFlag = ConfusionIgnore;
//            return p + sizeof(CONFUSION_BLOCK_END_FLAG) -1;
//        }
//        else
//        {
//            p++;
//        }
//    }
//    
//    return p;
//}

const char *MimiParser::ParsePreprocessor(const char *code)
{
    const char *p = code + 1;
    for (; *p; p++)
    {
        if ((*p == '\r' || *p == '\n') && (p[-1] != '\\'))
        {
            PrintOut("宏定义:", code, p + 1);
            return p + 1; //跳过空白
        }
    }
    return p;
}

// ' or " block
const char *MimiParser::ParseString(const char *code)
{
    char ch = *code;
    const char *p = code + 1;
    for (; *p; p++)
    {
        if (*p == '\\')
        {
            p++;
        }
        else if (*p == ch)
        {
            PrintOut("字符串:", code, p + 1);
            return p + 1;
        }
    }
    printf("字符串未正确结束：%s\n", code);
    return p;
}

//
const char *MimiParser::ParseComment(const char *code)
{
    const char *p = code + 2;
    for (; *p; p++)
    {
        if (*p == '\r' || *p == '\n')
        {
            PrintOut("单行注释:", code, p + 1);
            return p + 1;
        }
    }
    printf("单行注释未正确结束：%s\n", code);
    return p;
}

//
const char *MimiParser::ParseComments(const char *code)
{
    const char *p = code + 2;
    for (; *p; p++)
    {
        if (*p == '*' && p[1] == '/')
        {
            PrintOut("多行注释:", code, p + 2);
            return p + 2;
        }
    }
    printf("多行注释未正确结束： %s\n", code);
    return p;
}

//  Nest block () [] {} <>
template <char startch>
const char *MimiParser::ParseBlock(const char *code)
{
    char endch = (startch == '(') ? ')' : (startch + 2); // ascii 除了()+1 其他都是+2
    const char *p = code + 1; // 去掉括号本身
    while (*p)
    {
        if (const char *q = ParseCommon(p)) // 先处理过滤注释宏定义等
        {
            p = q;
        }
        else if (*p == endch) // 如果是括号结束符，就结束
        {
            return p + 1; // +1是因为这个函数返回的值在后续还要继续处理
        }
        else if (*p == startch) // 如果是开始符号，说明括号内还嵌套括号，递归处理
        {
            p = ParseBlock<startch>(p);
        }
        else // 否则处理括号内容
        {
            if (startch == '{')	// 如果是大括号内容，就忽略，Should be omitted by compiler optimization
            {
                p = ParseIgnore(p);
            }
            p++;
        }
    }
    printf("块未正确结束： %s\n", code);
    return p;
}

const char *MimiParser::ParseIgnore(const char *code)
{
    const char *p = code;
    if (!memcmp(p, "NSClassFromString", sizeof("NSClassFromString") - 1) ||
        !memcmp(p, "NSSelectorFromString", sizeof("NSSelectorFromString") - 1))
    {
        p += ((p[2] == 'C') ? (sizeof("NSClassFromString") - 1) : (sizeof("NSSelectorFromString") - 1));
        while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n' || *p == '(' || *p == '@') p++;
        if (*p == '"')
        {
            for (const char *symbol = ++p; *p; p++)	// Multiple arguments selector
            {
                if (*p == ':')
                {
                    if (!memcmp(symbol, "set", 3))	// Ignore setter
                    {
                        char ch = symbol[3];
                        if (ch >= 'A' && ch <= 'Z')
                        {
                            ((char *)symbol)[3] = tolower(ch);
                            PushSymbol(symbol + 3, unsigned(p - symbol - 3), MimiSymbolIgnore);
                            ((char *)symbol)[3] = ch;
                        }
                    }
                    PushSymbol(symbol, unsigned(p - symbol), MimiSymbolIgnore);
                    symbol = p + 1;
                }
                else if (*p == '"')
                {
                    PushSymbol(symbol, unsigned(p - symbol), MimiSymbolIgnore);
                    p++;
                    break;
                }
            }
        }
    }
    return p;
}

// @interface @implementation @protocol
const char *MimiParser::ParseObject(const char *code, MimiSymbolType type)
{
    const char *p = code;
    if(type == MimiSymbolInterface)
    {
        p += sizeof("@interface") - 1;
    }
    else if(type == MimiSymbolProtocol)
    {
        p += sizeof("@protocol") - 1;
    }
    else if(type == MimiSymbolImplementation)
    {
        p += sizeof("@implementation") - 1;
    }
    
//    p = ParseSolid(p); // 处理 @interface @implementation @protocol 本身
    p = ParseBlank(p); // 处理 @interface @implementation @protocol之后的空白
    p = ParseSymbol(p, type); // 处理@interface @implementation @protocol之后的名字，如@interface object中的object
    PrintOut("类:", code, p);
    
    while (*p)
    {
        if (const char *q = ParseCommon(p))
        {
            p = q;
        }
        else if (*p == '{')
        {
            p = ParseBlock(p);
        }
        else if (*p == '-' || *p == '+')
        {
            p = ParseMethod(p);
        }
        else if (!memcmp(p, "@property", sizeof("@property") - 1))
        {
            p = ParseProperty(p);
        }
        else if (!memcmp(p, "@end", sizeof("@end") - 1))
        {
            return p + 3 + 1; // 跳过@end到下一个字符
        }
        else
        {
            p++;
        }
    }
    printf("类、接口、协议未正确结束： %s\n", code);
    return p;
}

const char *MimiParser::ParseUntil(const char *p, char c)
{
    while (*p != c)
    {
        if (const char *q = ParseCommon(p))
            p = q;
        else if (*p)
            p++;
        else
            return p;
    }
    return p;
}

//
const char *MimiParser::ParseBlank(const char *p)
{
    while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')
    {
        if (*p == '/' && p[1] == '/')
            p = ParseComments(p);
        else if (*p == '/' && p[1] == '*')
            p = ParseComments(p);
        else if (*p == '\\')
            p += 2;
        else
            p++;
    }
    return p;
}

//
const char *MimiParser::ParseSolid(const char *p)
{
    while (*p && *p != ' ' && *p != '\t' && *p != '\r' && *p != '\n' && *p != ';' && *p != '{')
    {
        if (const char *q = ParseCommon(p))
            p = q;
        else
            p++;
    }
    return p;
}

// +/-
const char *MimiParser::ParseMethod(const char *code)
{
    const char *p = code + 1; // 跳过+ -
    p = ParseUntil(p, '(');
    p = ParseBlock<'('>(p);
    p = ParseBlank(p);
    const char *symbol = p;
    const char *first = symbol;
    while (*p)
    {
        if (const char *q = ParseCommon(p))
        {
            p = q;
        }
        else switch (*p)
        {
            case ';': // 结尾是;就是头文件的定义
                if (first == symbol) ParseMethodSymbol(symbol);
                PrintOut("方法定义:", code, p);
                return p + 1;
                
            case '{': // 结尾是{就是方法实现
                if (first == symbol) ParseMethodSymbol(symbol); //- (void)value:{},判断是否是setter方法
                PrintOut("方法实现:", code, p);
                return ParseBlock(p); // 过滤掉{}以及内容
                
            case ':': // 遇到：表示一个符号，如 abc:def:ghi:
                ParseMethodSymbol(symbol, first == symbol); // 如果是第一个符号如，abc，那么需要检查是否是setter方法
                p = ParseBlank(p + 1); // 过滤空白
                if (*p == '(') // 处理()以及两边空白
                {
                    p = ParseBlock<'('>(p);
                    p = ParseBlank(p);
                }
                p = ParseSolid(p); // 参数变量名过滤
                p = ParseBlank(p); // 过滤空白
                symbol = p; // 下一个符号开始地址
                break;
                
            default:
                p++;
                break;
        }
    }
    printf("方法体或定义未正确结束： %s\n", code);
    return p;
}

// @property (...) BOOL ** aa;
const char *MimiParser::ParseProperty(const char *code)
{
    MimiSymbolType type = MimiSymbolProperty;
    const char *p = code + sizeof("@property") - 1; // 跳过固定字符
    p = ParseBlank(p); // 跳过空白符
    if (*p == '(')
    {
        p++;
        while (*p)
        {
            if (*p == ')') // 如果是这个，说明括号将要结束
            {
                p = ParseBlank(p + 1); // 顺便去除括号后面的空白符号
                break;
            }
            else if (!memcmp(p, "getter", sizeof("getter") - 1) || !memcmp(p, "setter", sizeof("setter") - 1)) // 否则判断是否有getter和setter属性
            {
                MimiSymbolType type = (*p == 'g') ? MimiSymbolGetter : MimiSymbolSetter;
                p += sizeof("getter") - 1; // getter,setter长度一样
                p = ParseUntil(p, '='); // 等于的时候跳出来，即返回=
                p = ParseBlank(p + 1); // +1跳过=，然后处理空白符
                p = ParseSymbol(p, type); // 真正处理符号，会加入到符号表
            }
            else
            {
                if (!memcmp(p, "readonly", sizeof("readonly") - 1))
                {
                    type = MimiSymbolReadOnly;
                    p += sizeof("readonly") - 1;
                }
                else
                {
                    p++;
                }
                
            }
        }
    }
    
    p = ParseSolid(p); // 处理完空白后，处理变量类名
    while (*p)
    {
        if (const char *q = ParseCommon(p))
        {
            p = q;
        }
        else switch (*p)
        {
            case '<': // id<xxxxx>
                p = ParseBlock<'<'>(p);
                break;
                
            case '{':
                p = ParseBlock(p);
                break;
                
            case ' ':
            case '\t':
            case '\r':
            case '\n':
            case '*':
            case '&':
            case '(':
            case '^':
                p++; // void (^xxxx)() and so on
                break;
                
            default:
                p = ParseSymbol(p, type); // 解析符号并存储
                p = ParseUntil(p, ';'); // 处理结尾
                return p;
        }
    }
    return p;
}

//
const char *MimiParser::ParseMethodSymbol(const char *code, bool checkSetter)
{
    if (checkSetter && !memcmp(code, "set", 3)) // 如果是setter方法
    {
        char ch = code[3];
        if (ch >= 'A' && ch <= 'Z')
        {
            ((char *)code)[3] = tolower(ch); // 如果是setter，就将名字第一个字母小写，如：setValue ==> setvalue ，便于下面处理
            const char *p = ParseSymbol(code + 3, MimiSymbolProperty);
            ((char *)code)[3] = ch; // 还原小写字母
            return p;
        }
    }
    return ParseSymbol(code, MimiSymbolMethod); // 其他方法处理
}

//
const char *MimiParser::ParseSymbol(const char *code, MimiSymbolType type)
{
    const char *p = code;
    while ((*p >= '0' && *p <= '9') || (*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') || (*p == '_') || (*p == '$')) p++;
    
    if (m_confusionFlag == ConfusionOnce || m_confusionFlag == ConfusionAllInBlock || m_confusionFlag == ConfusionAllInFile) {
         PushSymbol(code, unsigned(p - code), type);
    }
    
    if (m_confusionFlag == ConfusionOnce) {
        m_confusionFlag = ConfusionIgnore;
    }
    
    return p;
}

void MimiParser::PushSymbol(const char *symbol, unsigned length, MimiSymbolType type)
{
    if (length == 0)
    {
        return;
    }
    
    // Skip duplicate
    for (std::vector<MimiSymbol>::iterator it = m_symbolVec.begin(); it != m_symbolVec.end(); ++it)
    {
        MimiSymbol &item = *it;
        if ((length == item.value.length()) && !memcmp(symbol, item.value.c_str(), length))
        {
            if (item.type != MimiSymbolProperty && item.type != MimiSymbolIgnore)
            {
                item.type = type;
            }
            return;
        }
    }
    
    MimiSymbol sym;
    sym.type = type;
    sym.value = string(symbol,0,length);
    m_symbolVec.push_back(sym);
}

const std::vector<MimiSymbol> &MimiParser::SymbolVec()
{
    return m_symbolVec;
}

void MimiParser::setDebug(bool debug)
{
    m_isDebug = debug;
}

void MimiParser::PrintOut(const char *type, const char *code, const char *end)
{
    if (m_isDebug) {
        cout<<type<<endl<<string(code,0,end - code)<<endl;
    }
}