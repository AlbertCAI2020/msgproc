#pragma once
#include <thread>
#include <mutex>
#include <condition_variable>
#include "log.h"
#include "message_source.h"

//消息处理系统
class MessageHandler
{
public:
    //消息处理系统初始化，需要指定消息源
    MessageHandler(MessageSource* msg_src)
    {
        msg_src_ = msg_src;
        stop_ = false;
        null_msg_count = 0;
    }

    //启动消息处理系统
    void start()
    {
        //向消息源注册一个消息通知回调函数，在该回调函数中，唤醒消息处理线程
        msg_src_->set_msg_notifier([this](){
            log() << "awake thread" << std::endl;
            awake_thread();
        });

        //启动消息处理线程
        msg_handle_thread_ = std::thread(&MessageHandler::run_in_thread, this);
    }   

    //停止消息处理系统
    void stop()
    {
        msg_src_->set_msg_notifier(nullptr);

        stop_ = true;
        awake_thread();
        msg_handle_thread_.join();        
    } 

private:
    //消息处理线程运行的函数，只要没有停止，就一直向消息源询问消息，如果一直没有消息，就阻塞自己，等待被唤醒之后继续轮询
    void run_in_thread()
    {
        while(!stop_)
        {
            std::unique_ptr<Message> msg = msg_src_->poll_msg();
           
            //如果询问返回了空指针
            if(nullptr == msg)
            {
                null_msg_count++;//累加连续空消息计数器

                //如果计数器不超过1000次，仅休眠5微秒
                if(null_msg_count < 1000)
                {
                    std::this_thread::sleep_for(std::chrono::microseconds(5));                    
                }
                else
                {
                    //如果连续空消息计数器累积了1000次以上（意味着至少5000微秒没有新消息）

                    null_msg_count = 0;//将计数器清零

                    //启用消息源的消息通知
                    msg_src_->enable_msg_notifier(true);
                    
                    log() << "block thread" << std::endl;
                    //阻塞消息处理线程                  
                    block_thread();

                    log() << "thread is awaked" << std::endl;
                }
            }
            else
            {
                //如果询问返回了非空的消息，也要将连续空消息计数器清零
                null_msg_count = 0;

                //对消息进行处理，这里仅仅打印消息内容 
                log() << msg->content << std::endl;
            }
        }
    }

    //阻塞线程，让线程在一个条件变量上等待
    void block_thread()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        condition_variable_.wait(lock);       
    }

    //唤醒线程，通过条件变量通知
    void awake_thread()
    {
        condition_variable_.notify_one();
    }

    MessageSource* msg_src_;
    uint32_t null_msg_count;//连续空消息计数器，表示消息轮询连续返回空指针的次数

    std::thread msg_handle_thread_;
    volatile bool stop_;

    std::mutex mutex_;
    std::condition_variable condition_variable_;
};
