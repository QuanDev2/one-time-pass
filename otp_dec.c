#include "utilities.h"

int main(int argc, char *argv[]) {
  int socketFD,
      portNumber;
  struct sockaddr_in serverAddress;
  char *fileName = argv[1],
       *keyName = argv[2],
       *fileContent = readFile(fileName),
       *keyContent = readFile(keyName),
       *passPhrase = "Steve Jobs?",
       *okRes = "iPhones!";

  fileContent[strcspn(fileContent, "\n")] = '\0';
  keyContent[strcspn(keyContent, "\n")] = '\0';
  verifyFileAndKey(fileContent, keyContent);

  if (argc < 4) {
    fprintf(stderr, "USAGE: %s hostname port\n", argv[0]);
    exit(0);
  }  // Check usage & args

  socketFD = createSocket(&serverAddress, &portNumber, argv[3]);
  if (socketFD < 0)
    error("CLIENT: ERROR opening socket");

  // Connect to server
  if (connect(socketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)  // Connect socket to address
    error("CLIENT: ERROR connecting");

  // handshake initialization
  sendData(passPhrase, socketFD);
  char *res = receiveData(socketFD);
  // if the res phrase is not correct
  if (strcmp(res, okRes) != 0) {
    fprintf(stderr, "Connection rejected on %d. otp_dec cannot connect to opt_enc_d\n", portNumber);
    exit(2);
  }

  sendData(fileContent, socketFD);

  sendData(keyContent, socketFD);

  res = receiveData(socketFD);

  printf("%s\n", res);

  // clean up
  close(socketFD);  // Close the socket
  free(res);
  free(fileContent);
  free(keyContent);
  return 0;
}
