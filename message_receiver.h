#pragma once
#include "message_source.h"
#include <queue>
#include <mutex>
#include <thread>

//消息接收系统，实现了消息源接口
class MessageReceiver : public MessageSource
{
public:
    MessageReceiver()
    {
        notifier_ = nullptr;
        is_notifier_enabled_ = false;
        stop_ = false;
    }

    //启动消息接收系统，创建消息接收线程
    void start()
    {
        msg_receive_thread_ = std::thread(&MessageReceiver::run_in_thread, this);
    }

    //停止消息接收系统
    void stop()
    {
        stop_ = true;
        msg_receive_thread_.join();
    }

    //实现消息源的接口:设置消息通知回调函数 notifier，当消息源有消息时，可以调用该notifier
    void set_msg_notifier(std::function<void()> notifier) override
    {
        notifier_ = notifier;
    }

    //实现消息源的接口:启用或者禁用消息通知，仅当启用时，消息源有消息才会调用消息通知回调函数
    void enable_msg_notifier(bool enable) override
    {
        is_notifier_enabled_ = enable;
    }

    //实现消息源的接口:消息轮询函数，消息处理系统可以使用该函数主动向消息源询问消息，如果没有消息，返回空指针
    std::unique_ptr<Message> poll_msg() override
    {
        std::unique_lock<std::mutex> lock(mutex_);

        //如果消息队列中没有消息，返回空指针
        if(msg_queue_.empty())
        {
            return nullptr;
        }
        else//否则返回消息队列中的第一个消息
        {
            std::unique_ptr<Message> msg = std::move(msg_queue_.front());
            msg_queue_.pop();
            return msg;
        }
    }

private:

    //消息接收线程运行的函数，只要没有停止，就一直从网络接收消息
    void run_in_thread()
    {
        while(!stop_)
        {
            //接收消息
            std::vector<std::unique_ptr<Message>> msgs = receive_msg_from_network();

            //将消息保存到消息队列
            std::unique_lock<std::mutex> lock(mutex_);
            for(int i = 0; i < msgs.size(); i++)
            {
                msg_queue_.push(std::move(msgs[i]));
            }
            lock.unlock();

            //如果消息通知机制被启用，就调用消息通知回调函数
            if(is_notifier_enabled_ && nullptr != notifier_)
            {
                notifier_();
            }
        }
    }

    //模拟从网络接收消息，每10秒接收到10个消息
    std::vector<std::unique_ptr<Message>> receive_msg_from_network()
    {
        //等待10秒钟
        std::this_thread::sleep_for(std::chrono::milliseconds(10000));

        //返回10个消息
        std::vector<std::unique_ptr<Message>> msgs;
        for(int i = 0; i < 10; i++)
        {
            std::unique_ptr<Message> msg = std::make_unique<Message>();
            msg->content = "hello world";
            msgs.push_back(std::move(msg));
        }        
        return msgs;
    }

    std::function<void()> notifier_; 
    volatile bool is_notifier_enabled_;

    std::mutex mutex_;
    std::queue<std::unique_ptr<Message>> msg_queue_;
    
    std::thread msg_receive_thread_;
    volatile bool stop_;
};
