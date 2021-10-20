#include <iostream>
#include <fstream>
#include <string>
#include <thread>

#include <json.hpp>

#include "repl-manager.hh"
#include "receiver-manager.hh"

#define REPL_MSG_FILEPATH "etc/repl-msg.txt"

#define LOOP_SLEEP_DURATION 100

ReplManager::ReplManager(Messenger& messenger,
                         std::shared_ptr<ReceiverManager> receiverManager)
    : MessageReceiver(messenger, managedTag, receiverManager) {
}

void
ReplManager::handleMessage(const int& srcNodeId,
                           const Message& receivedMessage,
                           const Messenger::Connection& connection) {
  std::unique_lock<std::mutex> lock(m_mutex);

  ReplCode code = receivedMessage.getCode<ReplCode>();

  switch (code) {
  case ReplCode::START: {
    m_hasStarted = true;
    break;
  }
  case ReplCode::SPEED_LOW: {
    m_speed = ReplCode::SPEED_LOW;
    break;
  }
  case ReplCode::SPEED_MEDIUM: {
    m_speed = ReplCode::SPEED_MEDIUM;
    break;
  }
  case ReplCode::SPEED_HIGH: {
    m_speed = ReplCode::SPEED_HIGH;
    break;
  }
  case ReplCode::CRASH: {
    m_hasCrashed = true;
    break;
  }
  case ReplCode::RECOVER: {
    m_hasCrashed = false;
    break;
  }
  }
}

void
getTrailingLine(std::ifstream& ifs, std::string& line) {
  std::getline(ifs, line);
  ifs.clear();
}

void
parseLine(const std::string& line,
          const Messenger& messenger,
          Message& receivedMessage,
          bool& messageReceived,
          int& dstNodeId) {
  if (line.empty() == false) {
    size_t delimiterPos = line.find(",");
    bool delimiterFound = delimiterPos != std::string::npos;
    if (delimiterFound == true || line == "shutdown") {
      std::string codeStr;

      if (delimiterFound == true) {
        std::string dstNodeIdStr = line.substr(0, delimiterPos);
        dstNodeId = std::stoi(dstNodeIdStr);
        codeStr = line.substr(delimiterPos + 1);
      } else {
        dstNodeId = messenger.getRank();
        codeStr = line;
      }

      if (0 <= dstNodeId && dstNodeId < messenger.getClusterSize()) {
        auto codeIte = replParseMap.find(codeStr);

        if (codeIte != replParseMap.end()) {
          ReplCode code = codeIte->second;
          messenger.setMessage(code, receivedMessage);

          messageReceived = true;
        }
      }
    }
  }
}

void
fetchMessageFromFile(std::ifstream& ifs,
                     const Messenger& messenger,
                     Message& receivedMessage,
                     bool& messageReceived,
                     int& dstNodeId) {
  std::string line;
  getTrailingLine(ifs, line);

  parseLine(line, messenger, receivedMessage, messageReceived, dstNodeId);
}

void
ReplManager::startReceiver() {
  std::ifstream ifs(REPL_MSG_FILEPATH);

  // TODO check file validity

  bool isUp = true;
  while (isUp == true) {
    int srcNodeId = m_messenger.getRank();
    Message receivedMessage;
    bool messageReceived = false;
    int dstNodeId;

    fetchMessageFromFile(
        ifs, m_messenger, receivedMessage, messageReceived, dstNodeId);

    if (messageReceived == true && dstNodeId == m_messenger.getRank()) {
      if (dstNodeId == m_messenger.getRank()) {
        this->handleMessage(srcNodeId, receivedMessage);
      }

      isUp = receivedMessage.getCodeInt() != 0;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(LOOP_SLEEP_DURATION));
  }

  ifs.close();

  m_receiverManager->stopReceiver(MessageTag::LEADER_ELECTION);
  m_receiverManager->stopReceiver(MessageTag::CONSENSUS);
  m_receiverManager->stopReceiver(MessageTag::FAILURE_DETECTION);
}

bool
ReplManager::hasStarted() {
  std::unique_lock<std::mutex> lock(m_mutex);

  return m_hasStarted;
}

bool
ReplManager::hasCrashed() {
  std::unique_lock<std::mutex> lock(m_mutex);

  return m_hasCrashed;
}

ReplCode
ReplManager::getSpeed() {
  std::unique_lock<std::mutex> lock(m_mutex);

  return m_speed;
}
