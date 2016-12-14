//
//  main.cpp
//  mimi
//
//  Created by lemon4ex on 16/4/27.
//  Copyright © 2016年 lemon4ex. All rights reserved.
//

#include <iostream>
#include "MimiParser.hpp"
#include <getopt.h>
#include <string>
//#include <fcntl.h>
#include <fstream>
#include <vector>

using namespace std;

void EchoAbout()
{
    cout<<"Mimi Preprocessor 1.0.1"<<endl
        <<"Symbol Confusion for Objective C/C++"<<endl
        <<"CopyRight(R) 2016, Lemon4ex, Aigudao.NET."<<endl<<endl;
}

void EchoHelp()
{
    cout<<"USAGE: mimi -o output_file -d input_dir -f input_file -p prefix_file [-D]"<<endl
        <<"       -o     --output       output symbols to file"<<endl
        <<"       -d     --dir          input .h/.m/.mm from dir[s]"<<endl
        <<"       -f     --file         input .h/.m/.mm file[s]"<<endl
        <<"       -h     --help         usage help"<<endl
        <<"       -v     --version      tool version"<<endl
        <<"       -p     --prefix       fixed prefix instead of random"<<endl
        <<"       -D     --debug        output debug info"<<endl
        <<endl
        <<"Confused Tag:"<<endl<<endl
        <<"       /*?--CONFUSED_ONCE--?*/       Only Confused once"<<endl
        <<"       /*?--CONFUSED_START--?*/      Confused block start"<<endl
        <<"       /*?--CONFUSED_END--?*/        Confused block end"<<endl<<endl
        <<"EXAMPLE:"<<endl<<endl
        <<"       mimi -o ./out.h -f ./classes/file.m"<<endl<<endl
        <<"       mimi -o ./out.h -f ./classes/file.m -p ./classes/prefix.txt"<<endl<<endl
        <<"       mimi -o ./out.h -d ./classes -f ./other/file.m"<<endl<<endl
        <<"       mimi -o ./out.h -d ./classes1 -d ./classes2 -d ./classes3 -f ./other/file1.m -f ./other/file2.m -f ./other/file3.m"<<endl<<endl
        <<"       mimi -o ./out.h -d ./classes1,/classes2,./classes3 -f ./other/file1.m,./other/file2.m,./other/file3.m"<<endl;
}

string RandomString()
{
    unsigned length = 16 + rand() % 16;
    const static char newSymbolChars[] = "abcdefghijklmnopqrstuvwxyz_ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    string randString;
    for (int i = 0; i < length; i++)
    {
        randString += newSymbolChars[rand() % (sizeof(newSymbolChars) - 1)];
    }
    
    return randString;
}

void MakeHeaderFile(const vector<MimiSymbol> &symbolVec ,const string &outPutPath,const vector<string> &prefixVec)
{
    const static struct {const char type[12]; const char *desc;} _comments[] =
    {
        {"/* IGNO */ ", "NSClassFromString/NSSelectorFromString Ignored Symbol"},
        {"/* METH */ ", "Method Symbol"},
        
        {"/* PROP */ ", "Property Symbol"},
        {"/* REAP */ ", "Readonly Property Symbol"},
        {"/* PSET */ ", "Property Setter Symbol"},
        {"/* PGET */ ", "Property Getter Symbol"},
        
        {"/* PROT */ ", "Protocol Symbol"},
        {"/* INTE */ ", "Interface Symbol"},
        {"/* IMPL */ ", "Implementation Symbol"},
    };
    
    ofstream os(outPutPath,ios::out);
    if (!os.good()) {
        cout<<"无法打开输出文件："<<outPutPath<<endl;
        return;
    }
    
    for (unsigned i = 0; i < sizeof(_comments) / sizeof(_comments[0]); i++)
    {
        os<<"//"<<_comments[i].type<<":"<<_comments[i].desc<<endl;
    }
    
    os<<endl;
    
    if (prefixVec.size() == 0) {
        for (MimiSymbol symbol : symbolVec)
        {
            switch (symbol.type) {
                case MimiSymbolProperty:
                {
                    string name = RandomString();
                    os<<_comments[symbol.type].type<<"#define "<<symbol.value<<" "<<name<<endl;
                    os<<_comments[symbol.type].type<<"#define "<<"_"<<symbol.value<<" "<<"_"<<name<<endl;
                }
                    break;
                case MimiSymbolImplementation:
                case MimiSymbolProtocol:
                case MimiSymbolInterface:
                case MimiSymbolGetter:
                case MimiSymbolSetter:
                case MimiSymbolMethod:
                case MimiSymbolReadOnly:
                {
                    string name = RandomString();
                    os<<_comments[symbol.type].type<<"#define "<<symbol.value<<" "<<name<<endl;
                }
                    break;
                default:
                    break;
            }
            
        }
    }
    else
    {
        for (int i = 0; i < symbolVec.size(); ++i) {
            string name = prefixVec[rand() % prefixVec.size()];
            switch (symbolVec[i].type) {
                case MimiSymbolProperty:
                {
                    os<<_comments[symbolVec[i].type].type<<"#define "<<symbolVec[i].value<<" "<<name<<i<<endl;
                    os<<_comments[symbolVec[i].type].type<<"#define "<<"_"<<symbolVec[i].value<<" "<<"_"<<name<<i<<endl;
                }
                    break;
                case MimiSymbolImplementation:
                case MimiSymbolProtocol:
                case MimiSymbolInterface:
                case MimiSymbolGetter:
                case MimiSymbolSetter:
                case MimiSymbolMethod:
                case MimiSymbolReadOnly:
                {
                    os<<_comments[symbolVec[i].type].type<<"#define "<<symbolVec[i].value<<" "<<name<<i<<endl;
                }
                    break;
                default:
                    break;
            }
        }
    }
    
    
    
    os.close();
}

/*?--ABSCFS*/
int main(int argc, const char * argv[]) {
    
    EchoAbout();
    
    if (argc < 2) {
        EchoHelp();
        return 0;
    }
    
    int isDebug = false;
    
    struct option opts[] = {
        {"help",    no_argument,       NULL, 'h'},
        {"version", no_argument,       NULL, 'v'},
        {"dir",  required_argument, NULL, 'd'},
        {"file",  required_argument, NULL, 'f'},
        {"output",  required_argument, NULL, 'o'},
        {"prefix",  required_argument, NULL, 'p'},
        {"debug",  no_argument, &isDebug, 'D'},
    };
    
    int option = 0;
    int optIndex = 0;
    
    vector<string> dirPathVec;
    vector<string> filePathVec;
    string outPutPath;
    vector<string> prefixVec;
    
    while ((option = getopt_long(argc, (char * const *)argv, "hvd:f:o:p:D", opts, &optIndex)) != -1) {
        switch (option) {
            case 'h':
            {
                EchoHelp();
            }
                break;
            case 'v':
            {
                EchoAbout();
            }
                break;
            case 'd':
            {
                const char * split = ",";
                char *sub = strtok(optarg, split);
                while (sub) {
                    dirPathVec.push_back(sub);
                    sub = strtok(NULL, split);
                }
            }
                break;
            case 'f':
            {
                const char * split = ",";
                char *sub = strtok(optarg, split);
                while (sub) {
                    filePathVec.push_back(sub);
                    sub = strtok(NULL, split);
                }
            }
                break;
            case 'o':
            {
                outPutPath = optarg;
            }
                break;
            case 'p':
            {
                ifstream ifs(optarg);
                char buffer[1024];
                if (ifs.good()) {
                    while (!ifs.eof()) {
                        ifs.getline(buffer,sizeof(buffer));
                        if (strlen(buffer) != 0) {
                            prefixVec.push_back(buffer);
                        }
                    }
                }
                ifs.close();
            }
                break;
            case '?':
                cout<<"无效的选项字符 "<<optopt<<endl;
                break;
            case ':':
                cout<<opts[optIndex].name<<"缺少选项参数！"<<endl;
                break;
            default:
                EchoHelp();
                break;
        }
    }
    
    if (outPutPath.length() == 0) {
        cout<<"缺少输出路径参数 -o"<<endl;
        return 0;
    }
    
    MimiParser parser;
    parser.setDebug(isDebug);
    
    if (filePathVec.size() == 0 && dirPathVec.size() == 0) {
        cout<<"无输入文件或目录 -d or -f"<<endl;
        return 0;
    }
    
    for(string &file : filePathVec)
    {
        parser.Parse(file.c_str());
    }
    
    for(string &dir : dirPathVec)
    {
        parser.Parse(dir.c_str());
    }
    
    MakeHeaderFile(parser.SymbolVec(),outPutPath,prefixVec);
    
    
    return 0;
}
/*?ABECFS?--*/