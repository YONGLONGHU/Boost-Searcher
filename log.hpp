#pragma once
#include <iostream>
#include <string>
#include <ctime>
#include <cstdlib>

#define NORMAL 1
#define WARNING 2
#define DEBUG 3
#define FATAL 4

#define LOG(LEVEL, MESSAGE) log(#LEVEL, MESSAGE, __FILE__, __LINE__)

void log(const std::string& level, 
         const std::string& message,
         const char* file,
         int line) 
{
    // 获取格式化时间
    time_t now = time(nullptr);
    char timestamp[20];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&now));

    // 输出日志
    std::cout << "["
              << level << "] ["
              << timestamp << "] ["  // 添加时间格式化
              << message << "] ["
              << file << ":" 
              << line << "]" 
              << std::endl;
}
