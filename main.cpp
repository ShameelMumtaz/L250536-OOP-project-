#include "src/AcademicSystem.h"
#include <iostream>
#include <filesystem>
using namespace std;

int main() {
    
    filesystem::create_directories("data");

    cout << "\n";
    cout << "  ╔════════════════════════════════════════════╗" << endl;
    cout << "  ║   FAST Academic Office Automation System  ║" << endl;
    cout << "  ║   Replacing chaos with clean C++ code     ║" << endl;
    cout << "  ╚════════════════════════════════════════════╝" << endl;
    cout << "\n";

    AcademicSystem system("data/");
    system.run();

    return 0;
}
