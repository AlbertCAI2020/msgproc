#pragma once
#include <functional>
#include <memory>
#include "message.h"

//消息源接口
class MessageSource
{
public:
    //设置消息通知回调函数 notifier，当消息源有消息时，可以调用该notifier
    virtual void set_msg_notifier(std::function<void()> notifier) = 0;

    //启用或者禁用消息通知，仅当启用时，消息源有消息才会调用消息通知回调函数
    virtual void enable_msg_notifier(bool enable) = 0;

    //消息轮询函数，消息处理系统可以使用该函数主动向消息源询问消息，如果没有消息，返回空指针
    virtual std::unique_ptr<Message> poll_msg() = 0;
};
