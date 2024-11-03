#include "message_receiver.h"
#include "message_handler.h"


int main(int argc, char* argv[])
{
    //启动消息接收系统
    MessageReceiver receiver;
    receiver.start();

    //启动消息处理系统，消息接收系统作为消息源
    MessageHandler handler(&receiver);
    handler.start();

    //等待用户输入
    getchar();

    //停止消息接收与处理
    receiver.stop();
    handler.stop();

    return 0;

};
