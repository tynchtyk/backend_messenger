#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <algorithm>

#include <iostream>
#include "helper.h"
#include "User.h"

using namespace std;

vector <User> userPool;
pthread_mutex_t vec_access_mut;

bool isAvailableUsername (string username) {
    pthread_mutex_lock(&vec_access_mut);
    bool is = true;
    for (User user : userPool) {
      if (user.username == username) {
        is = false;
        break;
      }
    }
    pthread_mutex_unlock(&vec_access_mut);
    return is;
}

User addToUserPool(int client_id, int room, string username) {
    User user(client_id, room, username);
    pthread_mutex_lock(&vec_access_mut);
    userPool.push_back(user);
    pthread_mutex_unlock(&vec_access_mut);
    return user;
}

User changeUserRoom(int client, int newRoom) {
  pthread_mutex_lock(&vec_access_mut);
  User foundUser;
  for (User &user : userPool) {
    if (user.client_id == client)
    {
      user.room = newRoom;
      foundUser = user;
      break;
    }
  }
  pthread_mutex_unlock(&vec_access_mut);
  // throw "[changeUserRoom] Can't find username with given id = " + to_string(client);
  return foundUser;
}

string getAvailableUsername(string username) {
  // check for duplicates
  if (isAvailableUsername(username))
    return username;
  for (int count = 2 ; /*assumed it will stop anyway*/ ; count ++) {
    string newUsername = username + "-" + to_string(count);
    if (isAvailableUsername(newUsername))
      return newUsername;
  }
  return username; // never happens
}

User getUserFromPool (int client_id) {
  pthread_mutex_lock(&vec_access_mut);
  User foundUser;
  for (User user : userPool) {
    if (user.client_id == client_id) {
      foundUser = user;
      break;
    }
  }
  pthread_mutex_unlock(&vec_access_mut);
  // throw "[getUserFromPool] Can't find the user with given id = " + to_string(client_id);
  return foundUser;
}

void announceJoinStatus(User connectedUser) {
  string userMessage = "Hello "
                        + connectedUser.username
                        + " This is room #"
                        + to_string(connectedUser.room);
  string roomMessage = connectedUser.username
                        + " joined room "
                        + to_string(connectedUser.room);

  pthread_mutex_lock(&vec_access_mut);
  for (User user : userPool) {
    if (user.client_id == connectedUser.client_id) {
      send(user.client_id, userMessage.c_str(), strlen(userMessage.c_str()), 0);
    } else if (user.room == connectedUser.room) {
      send(user.client_id, roomMessage.c_str(), strlen(roomMessage.c_str()), 0);
    }
  }
  pthread_mutex_unlock(&vec_access_mut);
}

void announceUserLeave(User leftUser) {
  string userMessage = "Good Bye "
                        + leftUser.username;
  string roomMessage = leftUser.username
                        + " is disconnected from room #"
                        + to_string(leftUser.room);
  pthread_mutex_lock(&vec_access_mut);
  for (User user : userPool) {
    if (user.client_id == leftUser.client_id) {
      send(user.client_id, userMessage.c_str(), strlen(userMessage.c_str()), 0);
    } else if (user.room == leftUser.room) {
      send(user.client_id, roomMessage.c_str(), strlen(roomMessage.c_str()), 0);
    }
  }
  userPool.erase(remove(userPool.begin(), userPool.end(),
            leftUser), userPool.end());
  pthread_mutex_unlock(&vec_access_mut);
}

void needInfo(User user) {
    string info = "[YOUR NAME] " + user.username + "\n" +
                  "[YOUR ROOM]" + to_string(user.room) + "\n" +
                  "[YOUR CLIENT ID]" + to_string(user.client_id) + "\n";
    send(user.client_id, info.c_str(), strlen(info.c_str()), 0);
}

void announceRoomUserList(User requestUser) {
  pthread_mutex_lock(&vec_access_mut);

  int ind = 1; string usersList = "";
  for (User user : userPool) {
    if (user.room == requestUser.room) {
      usersList += to_string(ind) + ". " + user.username + "\n";
      ind ++;
    }
  }
  pthread_mutex_unlock(&vec_access_mut);

  send(requestUser.client_id,
       usersList.c_str(), strlen(usersList.c_str()), 0);
}

bool existInRoom(string username, User user, User &foundUser) {
  pthread_mutex_lock(&vec_access_mut);
  bool found = false;
  for (User u : userPool) {
    if (u.room == user.room) {
      if (u.username == username) {
        foundUser = u;
        found = true;
        break;
      }
    }
  }
  pthread_mutex_unlock(&vec_access_mut);
  return found;
}

void *threadMsging(void *data) {
  return NULL;
}

void messageToAll(string msg, User user) {
  msg = user.username + ": " + msg;
  for (User u : userPool) {
    if (u.room == user.room && u.client_id != user.client_id) {
      send(u.client_id, msg.c_str(), strlen(msg.c_str()), 0);
    }
  }
}

void messageToUsers(std::vector<string> messagedUsers, string msg, User user) {
  vector< pair<int, User> > need_to_wait;
  vector< string > wrong_names;
  msg = user.username + ": " + msg;

  User foundUser;
  for (string mu : messagedUsers) {
      if (mu.find("#") != string::npos) {
        vector<string> parsed = split(mu, '#');
        if (existInRoom(parsed[0], user, foundUser)) {
          need_to_wait.push_back(make_pair(stoi(parsed[1]), foundUser));
        } else {
          wrong_names.push_back(parsed[0]);
        }
      } else {
        if (existInRoom(mu, user, foundUser)) {
          send(foundUser.client_id, msg.c_str(), strlen(msg.c_str()), 0);
        } else {
          wrong_names.push_back(mu);
        }
      }
  }

  sort(need_to_wait.begin(), need_to_wait.end());
  int wait = 0;
  for (pair<int, User> mu : need_to_wait) {
    wait = mu.first - wait;
    sleep(wait);
    send(mu.second.client_id, msg.c_str(), strlen(msg.c_str()), 0);
  }

  if (wrong_names.empty()) {
    return;
  }
  string wrong_user_msg = "There is no such a user: ";

  for (size_t in = 0; in < wrong_names.size(); in ++) {
    wrong_user_msg += wrong_names[in];
    if (in < wrong_names.size() - 1)
      wrong_user_msg += ",";
  }
  send(user.client_id, wrong_user_msg.c_str(), strlen(wrong_user_msg.c_str()), 0);
}

enum PROCESS_STATUS_TYPE {
    PROCESS_SUCCESS = 0,
    CLOSE_CLIENT,
    PROCESS_FAIL
};

PROCESS_STATUS_TYPE processTheRequest (string requestFromClient, int client) {
  vector <string> list = eraseSpaces(split(requestFromClient, ' '));
  CLIENT_REQUEST_TYPE type = stringToRequestType(list[0]);

  cout << "[CLIENT ID] " << client << "\n";
  cout << "[CLIENT REQUEST] " << requestFromClient << "\n";
  cout << "[TYPE] " << type <<"\n";

  if (type == REQUEST_CONNECT) {
    string username = getAvailableUsername(list[1]);
    int room = stoi(list[2]);
    User user = addToUserPool(client, room, username);
    announceJoinStatus(user);
    return PROCESS_SUCCESS;
  } else if (type == REQUEST_JOIN) {
    int newRoom = stoi(list[1]);
    User user = changeUserRoom(client, newRoom);
    announceJoinStatus(user);
    return PROCESS_SUCCESS;
  } else if (type == REQUEST_QUIT) {
    User user = getUserFromPool(client);
    announceUserLeave(user);
    return CLOSE_CLIENT;
  } else if (type == REQUEST_LIST_USERS) {
    User user = getUserFromPool(client);
    announceRoomUserList(user);
    return PROCESS_SUCCESS;
  } else if (type == REQUEST_INFO) {
    User user = getUserFromPool(client);
    needInfo(user);
    return PROCESS_SUCCESS;
  } else {
    // parsing the message :(
    User user = getUserFromPool(client);
    vector<string> messagedUsers
      = split(requestFromClient, ',');
    vector<string> last_two = split(messagedUsers.back(), ':');
    messagedUsers.pop_back();
    messagedUsers.push_back(last_two.front());
    messagedUsers = eraseSpaces(messagedUsers);

    string Msg = last_two.back();

    cout << "[USERS]\n";
    for (string ms : messagedUsers) {
      cout << ms << "\n";
    }
    cout << "[MESSAGE] " << Msg << "\n";
    if (messagedUsers.size() == 1 && messagedUsers[0] == "All") {
      messageToAll(Msg, user);
    } else {
      messageToUsers(messagedUsers, Msg, user);
    }
    //cout << "[NOTHING HAPPENED]\n";
  }
  return PROCESS_FAIL;
}

void *listenToClient (void* client_p) {
  int client = (long) client_p;
  int buflen = 1024;
  char *buf = new char[buflen+1];

  cout << "[CONNECTED ] client = " << client << "\n";
  while (1)
  {
        // read a request
      cout << "[READING] client = " << client << "\n";
      memset(buf, 0, buflen);
      int nread = recv(client,buf,buflen,0);
      if (nread <= 0) {
        User user = getUserFromPool(client);
        announceUserLeave(user);
        break;
      }

      string requestFromClient = (string) buf;

      PROCESS_STATUS_TYPE st = processTheRequest(requestFromClient, client);
      if (st == CLOSE_CLIENT) {
        break;
      }
      cout << "\n\n";
  }
  cout << "[DISCONNECTING] with client = " << client << "\n";
  close(client);
  delete [] buf;
  return NULL;
}

pthread_t threads[10000];

int
main(int argc, char **argv)
{
    pthread_mutex_init(&vec_access_mut, NULL);

    struct sockaddr_in server_addr,client_addr;
    socklen_t clientlen = sizeof(client_addr);
    int port, reuse;
    int server, client;

    // setup default arguments
    port = 3000;

    // process command line options using getopt()
    // see "man 3 getopt"

      // setup socket address structure
    memset(&server_addr,0,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

      // create socket
    server = socket(PF_INET,SOCK_STREAM,0);
    if (!server) {
        perror("socket");
        exit(-1);
    }

      // set socket to immediately reuse port when the application closes
    reuse = 1;
    if (setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        perror("setsockopt");
        exit(-1);
    }

      // call bind to associate the socket with our local address and
      // port
    if (bind(server,(const struct sockaddr *)&server_addr,sizeof(server_addr)) < 0) {
        perror("bind");
        exit(-1);
    }

      // convert the socket to listen for incoming connections
    if (listen(server,SOMAXCONN) < 0) {
        perror("listen");
        exit(-1);
    }

    int num = 0;

    // accept clients
    while ((client = accept(server,(struct sockaddr *)&client_addr,&clientlen)) > 0) {
          int create_res = pthread_create(&threads[num ++], NULL, listenToClient, (void *) client);
          if (create_res) {
            cout << "Error creating the thread for client = " << client << "\n";
          }

    }

    close(server);
}
