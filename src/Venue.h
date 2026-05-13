#pragma once
#include <string>
#include <iostream>
using namespace std;


class Venue {
private:
    string roomID;
    int    capacity;
    bool   hasComputers;

public:
    Venue() : capacity(0), hasComputers(false) {}
    Venue(const string& id, int cap, bool comp)
        : roomID(id), capacity(cap), hasComputers(comp) {}

    string getRoomID()      const { return roomID; }
    int    getCapacity()    const { return capacity; }
    bool   getHasComputers()const { return hasComputers; }

    void setRoomID(const string& id) { roomID = id; }
    void setCapacity(int c)          { capacity = c; }
    void setHasComputers(bool b)     { hasComputers = b; }

    void display() const {
        cout << "  Venue: " << roomID
             << " | Capacity: " << capacity
             << " | Computers: " << (hasComputers ? "Yes" : "No") << endl;
    }
};
