#include "client.hh"



int
main(int argc, char* argv[]) {
  Client client{};

  client.connect(argc, argv);

  client.replicateCommands();

  client.disconnect();

  return 0;
}

