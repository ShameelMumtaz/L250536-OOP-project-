#pragma once
#include <string>
#include <iostream>
using namespace std;


class AcademicEntity {
protected:
    string ID;
    string name;
    string email;

public:
    AcademicEntity() {}
    AcademicEntity(const string& id, const string& n, const string& e)
        : ID(id), name(n), email(e) {}

    virtual ~AcademicEntity() {}

    // Pure virtual — every subclass must implement its own display
    virtual void displayProfile() const = 0;

    // Getters
    string getID()    const { return ID; }
    string getName()  const { return name; }
    string getEmail() const { return email; }

    // Setters
    void setID(const string& id)       { ID = id; }
    void setName(const string& n)      { name = n; }
    void setEmail(const string& e)     { email = e; }
};
