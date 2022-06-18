#pragma once
/////////////////////////////////////////////////////////////////////
// Comm.h - message-passing communication facility                 //
// ver 1.0                                                         //
// Jim Fawcett, CSE687-OnLine Object Oriented Design, Fall 2017    //
/////////////////////////////////////////////////////////////////////
/*
*  Package Operations:
*  -------------------
*  This package defines Sender and Receiver classes.
*  - Sender uses a SocketConnecter and supports connecting to multiple
*    sequential endpoints and posting messages.
*  - Receiver uses a SocketListener which returns a Socket on connection.
*  It also defines a Comm class
*  - Comm simply composes a Sender and a Receiver, exposing methods:
*    postMessage(Message) and getMessage()
*
*  Required Files:
*  ---------------
*  Comm.h, Comm.cpp,
*  Sockets.h, Sockets.cpp,
*  Message.h, Message.cpp,
*  Utilities.h, Utilities.cpp
*
*  Maintenance History:
*  --------------------
*  ver 1.0 : 03 Oct 2017
*  - first release
*/
#include "Message.h"
#include "BlockingQueue.h"
#include "Sockets.h"
#include <string>
#include <thread>

using namespace Sockets;

namespace MsgPassingCommunication
{
  ///////////////////////////////////////////////////////////////////
  // Receiver class

  class Receiver
  {
  public:
    Receiver(EndPoint ep, const std::string& name = "Receiver");
    template<typename CallableObject>
    void start(CallableObject& co);
    void stop();
    Message getMessage();
    BlockingQueue<Message>* queue();
  private:
    BlockingQueue<Message> rcvQ;
    SocketListener listener;
    std::string rcvrName;
  };

  //BlockingQueue<Message> Receiver::rcvQ;

  ///////////////////////////////////////////////////////////////////
  // Sender class

  class Sender
  {
  public:
    Sender(const std::string& name = "Sender");
    ~Sender();
    void start();
    void stop();
    bool connect(EndPoint ep);
    void postMessage(Message msg);
    bool sendFile(const std::string& fileName);
  private:
    BlockingQueue<Message> sndQ;
    SocketConnecter connecter;
    std::thread sendThread;
    EndPoint lastEP;
    std::string sndrName;
  };

  class Comm
  {
  public:
    Comm(EndPoint ep, const std::string& name = "Comm");
    void start();
    void stop();
    void postMessage(Message msg);
    Message getMessage();
    std::string name();
    bool connect(EndPoint ep);

  private:
    Sender sndr;
    Receiver rcvr;
    std::string commName;
  };

  //----< callable object posts incoming message to rcvQ >-------------
/*
*  This is ClientHandler for receiving messages and posting
*  to the receive queue.
*/
  class ClientHandler
  {
  public:
      //----< acquire reference to shared rcvQ >-------------------------

      ClientHandler(BlockingQueue<Message>* pQ, const std::string& name = "clientHandler") : pQ_(pQ), clientHandlerName(name)
      {
          BOOST_LOG_TRIVIAL(info) <<("starting ClientHandler");
      }
      //----< shutdown message >-----------------------------------------

      ~ClientHandler()
      {
          BOOST_LOG_TRIVIAL(info) <<("ClientHandler destroyed;");
      }
      //----< set BlockingQueue >----------------------------------------

      void setQueue(BlockingQueue<Message>* pQ)
      {
          pQ_ = pQ;
      }
      //----< frame message string by reading bytes from socket >--------

      std::string readMsg(Socket& socket)
      {
          std::string temp, msgString;
          while (socket.validState())
          {
              temp = socket.recvString('\n');  // read attribute
              msgString += temp;
              if (temp.length() < 2)           // if empty line we are done
                  break;
          }
          return msgString;
      }
      //----< reads messages from socket and enQs in rcvQ >--------------

      void operator()(Socket socket)
      {
          while (socket.validState())
          {
              std::string msgString = readMsg(socket);
              if (msgString.length() == 0)
              {
                  // invalid message
                  break;
              }
              Message msg = Message::fromString(msgString);
              BOOST_LOG_TRIVIAL(info) <<(clientHandlerName + " RecvThread read message: " + msg.name());
              //std::cout << "\n  -- " + clientHandlerName + " RecvThread read message: " + msg.name();
              pQ_->enQ(msg);
              //std::cout << "\n  -- message enqueued in rcvQ";
              if (msg.command() == "quit")
                  break;
          }
          BOOST_LOG_TRIVIAL(info) <<("terminating ClientHandler thread");
      }
  private:
      BlockingQueue<Message>* pQ_;
      std::string clientHandlerName;
  };
}