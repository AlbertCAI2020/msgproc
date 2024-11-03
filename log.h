#pragma once
#include <iostream>
#include <iomanip>
#include <chrono>

inline std::ostream& log()
{
    
    // 获取当前时间点
    auto now = std::chrono::system_clock::now();

    // 将时间点转换为当前时区的时间结构
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);

    // 将时间结构转换为本地时间
    std::tm* local_time = std::localtime(&now_c);

    // 获取微秒部分
    auto now_us = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()) % 1000000;

    // 打印时间
    std::cout << std::setfill('0') << std::setw(2) << local_time->tm_hour << ":"
    << std::setfill('0') << std::setw(2) << local_time->tm_min << ":"
    << std::setfill('0') << std::setw(2) << local_time->tm_sec << "."
    << std::setfill('0') << std::setw(6) << now_us.count() << " ";
    return std::cout;    
}
