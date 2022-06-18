#include <iostream>
#include <functional>
#include <conio.h>
#include "Comm.h"
#include "Logger.h"
#include "Utilities.h"
#include "Message.h"
#include "BlockingQueue.h"

using namespace  MsgPassingCommunication;
using SUtils = Utilities::StringHelper;

/////////////////////////////////////////////////////////////////////
// Test #1 - Demonstrates Sender and Receiver operations

void DemoSndrRcvr(const std::string& machineName)
{
    SUtils::title("Demonstrating Sender and Receiver classes");

    SocketSystem ss;
    MsgPassingCommunication::EndPoint ep1;
    ep1.port = 9091;
    ep1.address = "localhost";
    MsgPassingCommunication::Receiver rcvr1(ep1);
    BlockingQueue<Message>* pQ1 = rcvr1.queue();

    ClientHandler ch1(pQ1);
    rcvr1.start(ch1);

    EndPoint ep2;
    ep2.port = 9092;
    ep2.address = "localhost";
    MsgPassingCommunication::Receiver rcvr2(ep2);
    BlockingQueue<Message>* pQ2 = rcvr2.queue();

    ClientHandler ch2(pQ2);
    rcvr2.start(ch2);

    Sender sndr;
    sndr.start();
    bool connected = sndr.connect(ep1);
    Message msg;
    msg.name("msg #1");
    msg.to(ep1);
    msg.from(msg.to());
    msg.command("do it");
    msg.attribute("bodyAttrib", "zzz");
    StaticLogger<1>::flush();
    std::cout << "\n  sndr in main posting message:  " << msg.name();
    sndr.postMessage(msg);

    msg.name("msg #2");
    msg.to(EndPoint(machineName, 9092));
    StaticLogger<1>::flush();
    std::cout << "\n  sndr in main posting message:  " << msg.name();
    sndr.postMessage(msg);

    Message rcvdMsg = rcvr1.getMessage();  // blocks until message arrives
    StaticLogger<1>::flush();
    std::cout << "\n  rcvr1 in main received message: " << rcvdMsg.name();
    rcvdMsg.show();

    rcvdMsg = rcvr2.getMessage();  // blocks until message arrives
    StaticLogger<1>::flush();
    std::cout << "\n  rcvr2 in main received message: " << rcvdMsg.name();
    rcvdMsg.show();

    SUtils::title("Sending message to EndPoint that doesn't exist");

    msg.name("msg #3");
    msg.to(EndPoint("DoesNotExist", 1111));  // Unknown endpoint - should fail
    StaticLogger<1>::flush();
    std::cout << "\n  sndr in main posting message:  " << msg.name();
    msg.show();
    sndr.postMessage(msg);                   // will never reach rcvr

    msg.name("msg #4");
    msg.to(EndPoint("localhost", 9091));
    StaticLogger<1>::flush();
    std::cout << "\n  sndr in main posting message:  " << msg.name();
    sndr.postMessage(msg);                  // this should succeed
    StaticLogger<1>::flush();
    rcvdMsg = rcvr1.getMessage();
    std::cout << "\n  rcvr1 in main received message: " << rcvdMsg.name();
    rcvdMsg.show();

    rcvr1.stop();
    rcvr2.stop();
    sndr.stop();
    StaticLogger<1>::flush();

    std::cout << "\n  press enter to quit DemoSndrRcvr";
    _getche();
    std::cout << "\n";
}

/////////////////////////////////////////////////////////////////////
// Test #2 - Demonstrates Comm class using a single thread
//           sending and receiving messages from two Comm
//           instances.

void DemoCommClass(const std::string& machineName)
{
    SUtils::title("Demonstrating Comm class");

    SocketSystem ss;

    MsgPassingCommunication::EndPoint ep1("localhost", 9191);
    MsgPassingCommunication::Comm comm1(ep1, "comm1");
    comm1.start();

    MsgPassingCommunication::EndPoint ep2("localhost", 9192);
    MsgPassingCommunication::Comm comm2(ep2, "comm2");
    comm2.start();

    // send msg from comm1 to comm1
    Message msg;
    msg.name("msg #1");
    msg.to(ep1);
    msg.from(ep1);
    StaticLogger<1>::flush();
    std::cout << "\n  comm1 in main posting message:   " << msg.name();
    comm1.postMessage(msg);
    msg = comm1.getMessage();
    StaticLogger<1>::flush();
    std::cout << "\n  comm1 in main received message:  " << msg.name();
    msg.show();

    // send msg from comm1 to comm2
    msg.name("msg #2");
    msg.from(ep1);
    msg.to(ep2);
    StaticLogger<1>::flush();
    std::cout << "\n  comm1 in main posting message:  " << msg.name();
    comm1.postMessage(msg);
    msg = comm2.getMessage();
    StaticLogger<1>::flush();
    std::cout << "\n  comm2 in main received message: " << msg.name();
    msg.show();

    // send msg from comm2 to comm1
    msg.name("msg #3");
    msg.to(ep1);
    msg.from(ep2);
    StaticLogger<1>::flush();
    std::cout << "\n  comm2 in main posting message:  " << msg.name();
    comm2.postMessage(msg);
    msg = comm1.getMessage();
    StaticLogger<1>::flush();
    std::cout << "\n  comm1 in main received message: " << msg.name();
    msg.show();

    // send msg from comm2 to comm2
    msg.name("msg #4");
    msg.from(ep2);
    msg.to(ep2);
    StaticLogger<1>::flush();
    std::cout << "\n  comm2 in main posting message:  " << msg.name();
    comm2.postMessage(msg);
    msg = comm2.getMessage();
    StaticLogger<1>::flush();
    std::cout << "\n  comm2 in main received message: " << msg.name();
    msg.show();

    comm1.stop();
    comm2.stop();
    StaticLogger<1>::flush();
    std::cout << "\n  press enter to quit DemoComm";
    _getche();
}
/////////////////////////////////////////////////////////////////////
// Test #3 - Demonstrate server with two concurrent clients
//           sending and receiving messages

//----< handler for first concurrent client >------------------------

void ThreadProcClnt1()
{
    Comm comm(EndPoint("localhost", 9891), "client1Comm");
    comm.start();
    EndPoint serverEP("localhost", 9890);
    EndPoint clientEP("localhost", 9891);
    size_t IMax = 3;
    for (size_t i = 0; i < IMax; ++i)
    {
        Message msg(serverEP, clientEP);
        msg.name("client #1 : msg #" + Utilities::Converter<size_t>::toString(i));
        std::cout << "\n  " + comm.name() + " posting:  " << msg.name();
        comm.postMessage(msg);
        Message rply = comm.getMessage();
        std::cout << "\n  " + comm.name() + " received: " << rply.name();
        ::Sleep(100);
    }
    ::Sleep(200);
    Message stop;
    stop.name("stop");
    stop.to(serverEP);
    stop.command("stop");
    comm.postMessage(stop);
}
//----< handler for 2nd concurrent client >--------------------------

void ThreadProcClnt2()
{
    Comm comm(EndPoint("localhost", 9892), "client2Comm");
    comm.start();
    EndPoint serverEP("localhost", 9890);
    EndPoint clientEP("localhost", 9892);
    size_t IMax = 3;
    for (size_t i = 0; i < IMax; ++i)
    {

        Message msg(serverEP, clientEP);
        msg.name("client #2 : msg #" + Utilities::Converter<size_t>::toString(i));
        std::cout << "\n  " + comm.name() + " posting:  " << msg.name();
        comm.postMessage(msg);
        Message rply = comm.getMessage();
        std::cout << "\n  " + comm.name() + " received: " << rply.name();
    }
}
//----< server demonstrates two-way asynchronous communication >-----
/*
*  - One server receiving messages and sending replies to
*    two concurrent clients.
*/
void DemoClientServer()
{
    SUtils::title("Demonstrating Client-Server - one server with two concurrent clients");

    SocketSystem ss;

    EndPoint serverEP("localhost", 9890);
    EndPoint clientEP("localhost", 9891);
    Comm comm(serverEP, "serverComm");
    comm.start();
    std::thread t1(ThreadProcClnt1);
    t1.detach();
    std::thread t2(ThreadProcClnt2);
    t2.detach();

    Message msg, rply;
    rply.name("reply");
    size_t count = 0;
    while (true)
    {
        msg = comm.getMessage();
        std::cout << "\n  " + comm.name() + " received message: " << msg.name();
        //msg.show();
        rply.to(msg.from());
        rply.from(serverEP);
        rply.name("server reply #" + Utilities::Converter<size_t>::toString(++count) + " to " + msg.from().toString());
        //rply.show();
        comm.postMessage(rply);
        if (msg.command() == "stop")
        {
            break;
        }
    }
    comm.stop();
    StaticLogger<1>::flush();
    std::cout << "\n  press enter to quit DemoClientServer";
    _getche();
}

Cosmetic cosmetic;

int main()
{
  SUtils::Title("Demo of Message-Passing Communication");
  Utilities::putline();

  StaticLogger<1>::attach(&std::cout);

  ///////////////////////////////////////////////////////////////////
  // remove comment on line below to show many of the gory details
  //
  //StaticLogger<1>::start();

  ///////////////////////////////////////////////////////////////////
  // if you uncomment the lines below, you will run all demos

  DemoSndrRcvr("localhost");  // replace "Odin" with your machine name
  //DemoCommClass("Odin");
  //DemoClientServer();

  return 0;
}
