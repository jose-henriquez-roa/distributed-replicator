#pragma once

#include "messenger.hh"

class Client {
public:
  Client() = default;

  void
  connect(int argc, char** argv);

  void
  disconnect();

private:
  Messenger m_messenger;

  Messenger::Connection m_serverConnection;
};
