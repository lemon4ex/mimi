# mimi
一个用于扫描.h/.m/.mm文件，自动生成符号混淆宏定义文件的工具

时间 2016.5.6

### 使用：

```shell
USAGE: mimi -o output_file -d input_dir -f input_file -p prefix_file [-D]
       -o     --output       output symbols to file
       -d     --dir          input .h/.m/.mm from dir[s]
       -f     --file         input .h/.m/.mm file[s]
       -h     --help         usage help
       -v     --version      tool version
       -p     --prefix       fixed prefix instead of random
       -D     --debug        output debug info

EXAMPLE:

       mimi -o ./out.h -f ./classes/file.m

       mimi -o ./out.h -f ./classes/file.m -p ./classes/prefix.txt

       mimi -o ./out.h -d ./classes -f ./other/file.m

       mimi -o ./out.h -d ./classes1 -d ./classes2 -d ./classes3 -f ./other/file1.m -f ./other/file2.m -f ./other/file3.m

       mimi -o ./out.h -d ./classes1,/classes2,./classes3 -f ./other/file1.m,./other/file2.m,./other/file3.m

```

### 说明

xcode工程配合此工具进行混淆请参考念茜的博文：[ iOS安全攻防（二十三）：Objective-C代码混淆](http://blog.csdn.net/yiyaaixuexi/article/details/29201699)

她博文中使用的是手动列出需要混淆的方法和类名到一个文件中，比较繁琐。

使用此工具可以自动提取需要混淆的方法名和类名、属性名、协议名等，其他方案和她的雷同。

程序会扫描指定的目录或文件的内容，提取类名、方法名、属性名等符号，通过在需要混淆的符号前后定义如下注释，工具可自动提取符号并且生成混淆文件：

```c
const char *kConfusionOnceTag = "/*?--CONFUSED_ONCE--?*/"; ///< 只将此标记之后的一个标识符混淆
const char *kConfusionStartTag = "/*?--CONFUSED_START--?*/"; ///< 混淆块标记开始
const char *kConfusionEndTag = "/*?--CONFUSED_END--?*/"; ///< 混淆标记块结束
```

#### 一个栗子

```c
/*?--CONFUSED_ONCE--?*/ //只有这个标记后面的一个类名或方法名或属性名会被提取混淆，这里只混淆类名，后面的方法名等都不会提取混淆
@interface DataManager()<UpdateHintWindowDelegate>
{
    UpdateHintWindow *_updateHintWindow;
}

@end

@implementation DataManager


#pragma mark - init

/*?--CONFUSED_START--?*/ //这个范围内的类名，方法名，属性名，都会被提取混淆，这里只混淆两个方法名
+ (id)sharedInstance
{
    ...
}

- (id)init
{
    ...
}
/*?--CONFUSED_END--?*/

- (void)dealloc
{
    ...
}

@end
```

增加了以上标记的脚本使用工具跑完后，生成的宏定义头文件会包含三个符号定义，类似：

```
#define DataManager XFAEEsDEAFAeeafal
#define sharedInstance KKIxsedfkaowSDAFAF
#define init XKIJFEIKFIiioKDD
```
### 联系

1. @lemon4ex  

2. http://aigudao.net

3. admin@aigudao.net

4. netchen@vip.qq.com
