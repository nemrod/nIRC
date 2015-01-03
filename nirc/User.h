#ifndef USER_H
#define USER_H

#include <string>

using namespace std;

class User {
    public:
        User();
        bool setNick(string nick);
        string getNick();

    private:
        string nick;
};

#endif
