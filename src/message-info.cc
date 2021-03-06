#include "message.hh"

template <>
MessageTag
getTagFromCode<LeaderElectionCode>() {
  return MessageTag::LEADER_ELECTION;
}

template <>
MessageTag
getTagFromCode<ConsensusCode>() {
  return MessageTag::CONSENSUS;
}

template <>
MessageTag
getTagFromCode<ReplCode>() {
  return MessageTag::REPL;
}

template <>
MessageTag
getTagFromCode<FailureCode>() {
  return MessageTag::FAILURE_DETECTION;
}

template <>
MessageTag
getTagFromCode<ClientCode>() {
  return MessageTag::CLIENT;
}
