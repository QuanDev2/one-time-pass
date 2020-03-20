#include "utilities.h"

#define NUM_PROCESSES 5

char *encryptMsg(char *, char *);

int main(int argc, char *argv[]) {
  int listenSocketFD,
      establishedConnectionFD,
      portNumber,
      processCount = 0;
  socklen_t sizeOfClientInfo;
  struct sockaddr_in clientAddress;
  pid_t spawnidList[NUM_PROCESSES] = {-9};
  pid_t spawnid = -8;
  char *passPhrase = "Elon Musk?",
       *okPhrase = "SpaceX!",
       *rejectPhrase = "Wrong number";

  if (argc < 2) {
    fprintf(stderr, "USAGE: %s port\n", argv[0]);
    exit(1);
  }  // Check usage & args

  listenSocketFD = createSocket(&clientAddress, &portNumber, argv[1]);
  if (listenSocketFD < 0)
    error("ERROR opening socket");

  // Enable the socket to begin listening
  if (bind(listenSocketFD, (struct sockaddr *)&clientAddress, sizeof(clientAddress)) < 0)  // Connect socket to port
    error("ERROR on binding");

  listen(listenSocketFD, NUM_PROCESSES);  // Flip the socket on - it can now receive up to 5 connections

  pid_t parentPID = getpid(),
        zombiePID = 0;
  while (1) {
    switch (spawnid) {
      case -1:
        perror("Fork Error!\n");
        exit(1);
        break;
      case 0:;
        char *clientID = receiveData(establishedConnectionFD);
        // if passPhrase from client is not correct
        if (strcmp(clientID, passPhrase) != 0) {
          sendData(rejectPhrase, establishedConnectionFD);
          exit(0);
        } else {
          sendData(okPhrase, establishedConnectionFD);
        }
        char *msg = receiveData(establishedConnectionFD);
        char *key = receiveData(establishedConnectionFD);
        char *encryptedMsg = encryptMsg(msg, key);
        // Send a Success message back to the client
        sendData(encryptedMsg, establishedConnectionFD);
        // clean up
        free(msg);
        free(key);
        free(encryptedMsg);
        close(establishedConnectionFD);
        exit(0);
      default:;
        int i;
        sizeOfClientInfo = sizeof(clientAddress);
        establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo);
        if (establishedConnectionFD < 0)
          error("ERROR on accept");
        // if there's less than 6 process, fork(), else, wait

        if (processCount <= NUM_PROCESSES) {
          spawnid = fork();
        } else {
          zombiePID = wait(NULL);
        }
        if (getpid() == parentPID) {
          zombiePID = waitpid(-1, NULL, WNOHANG);
          if (zombiePID > 1) {
            // there's a zombie lurking
            processCount = processCount - 1;
          }
        }
        spawnidList[processCount] = spawnid;
        processCount = processCount + 1;

        break;
    }
  }

  close(listenSocketFD);  // Close the listening socket
  return 0;
}
