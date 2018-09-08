#ifndef USER_H
#define USER_H

class User{
public:
    int room, client_id;
    string username;
  //  bool active;

public:
    User() {

    }
    User(int client_id, int room, string username) {
      this->client_id = client_id;
      this->room = room;
      this->username = username;
//      this->active = true;
    }
    bool operator==(const User& other) const
    {
        if (this->client_id != other.client_id)
            return false;
        return true;
    }
    bool operator<(const User& other) const
    {
        if (this->client_id < other.client_id)
            return true;
        return false;
    }
};

#endif
