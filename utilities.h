#include <dirent.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

void sendData(char *, int);
void error(const char *);
int createSocket(struct sockaddr_in *, int *, char *);
void verifyFileAndKey(char *, char *);
bool hasBadChar(char *);
char *readFile(char *);
char *encryptMsg(char *, char *);
char *decryptMsg(char *, char *);

// read file with given filename
char *readFile(char *fileName) {
  // read plaintext file
  FILE *p_file = fopen(fileName, "r");
  if (p_file == NULL) {
    fprintf(stderr, "File not found\n");
  }
  char *fileContent;
  fseek(p_file, 0, SEEK_END);
  long fSize = ftell(p_file);
  fseek(p_file, 0, SEEK_SET);
  fileContent = calloc(fSize, sizeof(char));
  fread(fileContent, 1, fSize, p_file);
  return fileContent;
}

// check if string has bad chars
bool hasBadChar(char *str) {
  bool result = false;
  char *validString = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
  int i;
  for (i = 0; i < strlen(str); i++) {
    if (strchr(validString, str[i]) == NULL) {
      result = true;
      break;
    }
  }
  return result;
}

// verify if File Or Key has bad chars
void verifyFileAndKey(char *file, char *key) {
  if (strlen(key) < strlen(file)) {
    fprintf(stderr, "Error: Key file is way too short\n");
    exit(1);
  }
  if (hasBadChar(file) == true) {
    fprintf(stderr, "Error: text file has bad character(s)\n");
    exit(1);
  }
  if (hasBadChar(key) == true) {
    fprintf(stderr, "Error: Key file has bad character(s)\n");
    exit(1);
  }
}

// take a char* file and a socketFD, send file over socket
void sendData(char *file, int socketFD) {
  unsigned long charsSent = 0;
  uint32_t fileLen = strlen(file);

  char *remainingFile = file;
  int remainingChars = fileLen, totalCharsSent = 0;
  send(socketFD, &fileLen, sizeof(uint32_t), 0);
  // keep sending until totalSent == filelen
  while (totalCharsSent < fileLen) {
    charsSent = send(socketFD, remainingFile, remainingChars, 0);  // Write to the server
    if (charsSent < 0)
      error("CLIENT: ERROR writing to socket");
    remainingFile = remainingFile + charsSent;
    remainingChars = remainingChars - charsSent;
    totalCharsSent = totalCharsSent + charsSent;
  }
}
void error(const char *msg) {
  perror(msg);
  exit(1);
}  // Error function used for reporting issues

// take a socketFD, receiving data and return the string
char *receiveData(int socketFD) {
  // receiving the length first
  uint32_t fileLen;
  char buffer[1024];
  unsigned long r = recv(socketFD, &fileLen, sizeof(uint32_t), 0);
  if (r < 0)
    printf("Error receiving\n");
  char *msg = calloc(fileLen, sizeof(char));
  memset(msg, '\0', fileLen);
  unsigned long remaining = fileLen,
                charsRead = 0,
                totalRead = 0;
  // Get the message from the client and display it
  while (remaining > 0) {
    memset(buffer, '\0', 1024);
    if (remaining < 1024)
      charsRead = recv(socketFD, buffer, remaining, 0);
    else
      charsRead = recv(socketFD, buffer, 1023, 0);  // Read the client's message from the socket
    if (charsRead < 0)
      error("ERROR reading from socket");

    buffer[1024] = '\0';
    strcat(msg, buffer);
    totalRead = totalRead + charsRead;
    remaining = remaining - charsRead;
  }

  msg[totalRead] = '\0';
  return msg;
}

// create socket, return socketFD and change int* portNum
int createSocket(struct sockaddr_in *address, int *portNum, char *portStr) {
  *portNum = atoi(portStr);
  struct hostent *hostInfo;

  memset((char *)address, '\0', sizeof(*address));  // Clear out the address struct

  address->sin_port = htons(*portNum);
  address->sin_family = AF_INET;
  int socketFD;
  hostInfo = gethostbyname("localhost");  // Convert the machine name into a special form of address
  if (!hostInfo) {
    fprintf(stderr, "CLIENT: ERROR, no such host\n");
    exit(0);
  }
  memcpy((char *)&address->sin_addr.s_addr, (char *)hostInfo->h_addr_list[0], hostInfo->h_length);  // Copy in the address

  socketFD = socket(AF_INET, SOCK_STREAM, 0);

  return socketFD;
}

// Take a msg and and key, encrypt it
char *encryptMsg(char *msg, char *key) {
  unsigned long len = strlen(msg);
  char *result = calloc(len, sizeof(char));
  unsigned long i;
  for (i = 0; i < len; i++) {
    // convert spaces to [
    char msgChar = msg[i] == ' ' ? msgChar = '[' : msg[i];
    char keyChar = key[i] == ' ' ? keyChar = '[' : key[i];
    int temp = (msgChar - 65 + keyChar - 65) % 27;
    char letter = temp + 65;
    // convert it back to space
    if (letter == '[')
      letter = ' ';
    result[i] = letter;
  }
  return result;
}

// take a msg and a key, decrypt it
char *decryptMsg(char *msg, char *key) {
  unsigned long len = strlen(msg);
  char *result = calloc(len, sizeof(char));
  unsigned long i;
  for (i = 0; i < len; i++) {
    char letter = 0;
    // convert spaces to 91
    char cipherChar = msg[i] == ' ' ? cipherChar = '[' : msg[i];
    char keyChar = key[i] == ' ' ? keyChar = '[' : key[i];

    // perform decryption
    int temp = (cipherChar - keyChar) % 27;
    while (temp < 0)
      temp = temp + 27;
    if (temp == 26)
      letter = ' ';
    else
      letter = temp + 65;
    result[i] = letter;
  }
  return result;
}