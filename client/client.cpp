#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include "helper.h"

#include <fstream>
#include <iostream>
#include <string>

using namespace std;


string host = "127.0.0.1";
int port = 3000;
int room = 1;
string myNick = "Kadyr";
bool quit_request = false;

int server;

bool processArgs(int argc, char **argv) {
  if (argc < 4) {
			cout << "Not enough arguments\n";
			return false;
	}

	if(!parseIpPort(argv[1], host, port)) {
		cout << "Can't split first argument, IP and Port by \":\"\n";
		return false;
	}

	room = atoi(argv[2]);
	myNick = argv[3];

	return true;
}

void *listenUserInput(void *data) {
  string line;
  while (getline(cin,line)) {
      if (line.length() <= 0)
        continue;

      // write the data to the server
      send(server, line.c_str(), line.length(), 0);
      if (line.find("/quit") != string::npos) {
        quit_request = true;
        break;
      }
      // print the response
  }
  return NULL;
}

void *listenServerResponse(void *data) {
  int buflen = 1024;
  char* buf = new char[buflen+1];

  while (true) {
        if (quit_request) {
          // cout << "breaking the rules\n";
          break;
        }
        // read the response
        memset(buf,0,buflen);
        recv(server,buf,buflen,0);
        string received_msg = (string) buf;
        cout << received_msg << "\n\n";
  }
  delete[] buf;
  return NULL;

}
int
main(int argc, char **argv)
{
    // setup default arguments
    int option;
		(void)option;
    if (argc > 1 && !processArgs(argc, argv)) {
			return 0;
    }



      // setup socket address structure
    struct sockaddr_in server_addr;
    memset(&server_addr,0,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
		inet_aton(host.c_str(), &server_addr.sin_addr);

    server = socket(PF_INET,SOCK_STREAM,0);
    if (server < 0) {
        perror("socket");
        exit(-1);
    }

      // connect to server
    if (connect(server,(const struct sockaddr *)&server_addr,sizeof(server_addr)) < 0) {
        perror("connect");
        exit(-1);
    }

    // allocate buffer
    int buflen = 1024;
    char* buf = new char[buflen+1];

    // read a line from standard input

    string line = "/connect " + myNick + " " + std::to_string(room);


    // write the data to the server
    send(server, line.c_str(), line.length(), 0);
    memset(buf,0,buflen);
    recv(server,buf,buflen,0);

    // connection mesesage
    cout << buf << "\n";

    // thread IDs
    pthread_t listenUserInput_t;
    pthread_t listenServerResponse_t;

    // creating threads
    int create_res;

    create_res = pthread_create(&listenUserInput_t, NULL, listenUserInput, NULL);
    if (create_res) {
      cout << "Error occured while creating the thread for listenUserInput\n";
      return 0;
    }
    if (quit_request) {
      cout << "Need to quit\n";
    }
    create_res = pthread_create(&listenServerResponse_t, NULL, listenServerResponse, NULL);
    if (create_res) {
      cout << "Error occured while creating the thread for listenServerResponse\n";
      return 0;
    }



    // joining threads
    int join_res;
    void *thread_status;

    join_res = pthread_join(listenUserInput_t, &thread_status);
    if (join_res) {
      cout << "Unable to join listenUserInput" << join_res << "\n";
    } else {
    //  cout << "Successfully joined listenUserInput" << thread_status << "\n";
    }

    join_res = pthread_join(listenServerResponse_t, &thread_status);
    if (join_res) {
      cout << "Unable to join listenServerResponse" << join_res << "\n";
    } else {
    //  cout << "Successfully joined listenServerResponse" << thread_status << "\n";
    }

    // Close socket
    close(server);

    // main thread exiting
    pthread_exit(NULL);
}
