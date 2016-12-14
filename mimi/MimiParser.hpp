//
//  MimiParser.hpp
//  mimi
//
//  Created by lemon4ex on 16/4/27.
//  Copyright © 2016年 lemon4ex. All rights reserved.
//
//  Symbol Confusion for Objective C/C++\n"
//  Copyleft(L) 2014, Yonsm.NET

#ifndef MimiParser_hpp
#define MimiParser_hpp
#include <string>
#include <vector>

enum MimiSymbolType
{
    MimiSymbolIgnore,
    MimiSymbolMethod,
    
    MimiSymbolProperty,
    MimiSymbolReadOnly,
    MimiSymbolSetter,
    MimiSymbolGetter,
    
    MimiSymbolProtocol,
    MimiSymbolInterface,
    MimiSymbolImplementation,
};

struct MimiSymbol {
    MimiSymbolType type;
    std::string value;
};

enum ConfusionFlag
{
    ConfusionIgnore, ///< 忽略混淆，默认
    ConfusionOnce, ///< 只匹配一次混淆标记后面出现的符号
    ConfusionAllInBlock, ///< 匹配混淆标记开始与结束之间的所有符号
    ConfusionAllInFile ///< 混淆整个文件，忽略混淆标记
};

extern const char *CONFUSION_FLAG;

class MimiParser {
    std::vector<MimiSymbol> m_symbolVec;
    ConfusionFlag m_confusionFlag; // 混淆标志
    bool m_isDebug;
    
public:
    bool Parse(const char *arg);
    const std::vector<MimiSymbol> &SymbolVec();
    void setDebug(bool debug);
    
private:
    bool ParseDir(const char *dir);
    bool ParseFile(const char *file);
    bool ParseCode(const char *code);
    const char *ParseCommon(const char *code);
    
//    const char *ParseConfusionOnce(const char *code);
//    const char *ParseConfusionBlock(const char *code);
    // 宏定义
    const char *ParsePreprocessor(const char *code);
    // ' or " block
    const char *ParseString(const char *code);
    const char *ParseComment(const char *code);
    const char *ParseComments(const char *code);
    template <char startch = '{'>
    const char *ParseBlock(const char *code);
    const char *ParseIgnore(const char *code);
    const char *ParseObject(const char *code, MimiSymbolType type);
    // +/-
    const char *ParseMethod(const char *code);
    
    // @property (...) BOOL ** aa;
    const char *ParseProperty(const char *code);
    //
    const char *ParseMethodSymbol(const char *code, bool checkSetter = true);
    //
    const char *ParseSymbol(const char *code, MimiSymbolType type);
    const char *ParseUntil(const char *p, char c);
    //
    const char *ParseBlank(const char *p);
    //
    const char *ParseSolid(const char *p);
    void PushSymbol(const char *symbol, unsigned length, MimiSymbolType type);
    
    void PrintOut(const char *type, const char *code, const char *end);
};

#endif /* MimiParser_hpp */
