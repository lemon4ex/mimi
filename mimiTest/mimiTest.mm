//
//  mimiTest.m
//  mimiTest
//
//  Created by lemon4ex on 16/4/27.
//  Copyright © 2016年 lemon4ex. All rights reserved.
//

#import <XCTest/XCTest.h>
#include "MimiParser.hpp"
#include <iostream>

@interface mimiTest : XCTestCase

@end

@implementation mimiTest

- (void)setUp {
    [super setUp];
    // Put setup code here. This method is called before the invocation of each test method in the class.
}

- (void)tearDown {
    // Put teardown code here. This method is called after the invocation of each test method in the class.
    [super tearDown];
}

- (void)testParseCommon {
    // This is an example of a functional test case.
    // Use XCTAssert and related functions to verify your tests produce the correct results.
    const char *filePath = "/Users/lemon4ex/Desktop/mimi/mimiTest/common.txt";
    MimiParser parser;
    parser.Parse(filePath);
    
}

- (void)testParseObject {
    // This is an example of a functional test case.
    // Use XCTAssert and related functions to verify your tests produce the correct results.
    const char *filePath = "/Users/lemon4ex/Desktop/mimi/mimiTest/object.txt";
    MimiParser parser;
    parser.Parse("/Users/lemon4ex/Desktop/mimiTest/mimiTest/DetailViewController.m");
    
    for (MimiSymbol symbol : parser.SymbolVec()) {
        std::cout<<"类型："<<symbol.type<<" 值："<<symbol.value<<std::endl;
    }
}

- (void)testPerformanceExample {
    // This is an example of a performance test case.
    [self measureBlock:^{
        // Put the code you want to measure the time of here.
    }];
}

@end
