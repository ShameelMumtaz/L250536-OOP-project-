#include "src/AcademicSystem.h"
#include <iostream>
#include <filesystem>
using namespace std;

int main() {
    
    filesystem::create_directories("data");

    cout << "\n";

    cout << "   FAST Academic Office Automation System  " << endl;
   

    AcademicSystem system("data/");
    system.run();

    return 0;
}
