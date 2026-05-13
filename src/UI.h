#pragma once
#include <iostream>
#include <string>
#include <limits>
#include <climits>
using namespace std;


namespace UI {

    inline void banner(const string& title) {
        int w = 50;
        string line(w, '=');
        int pad = (w - (int)title.size() - 2) / 2;
        if (pad < 0) pad = 0;
        cout << "\n" << line << endl;
        cout << string(pad, ' ') << " " << title << " " << endl;
        cout << line << endl;
    }

    inline void divider(char c = '-', int w = 50) {
        cout << string(w, c) << endl;
    }

   
    inline string prompt(const string& msg) {
        cout << "  " << msg << ": ";
        string input;
        if (!getline(cin, input)) {
            // EOF: clear error bits and return empty string
            cin.clear();
            return "";
        }
        size_t a = input.find_first_not_of(" \t\r\n");
        size_t b = input.find_last_not_of(" \t\r\n");
        return (a == string::npos) ? "" : input.substr(a, b - a + 1);
    }

   
    inline int promptInt(const string& msg, int lo = INT_MIN, int hi = INT_MAX) {
        int attempts = 0;
        while (true) {
            string raw = prompt(msg);
            if (raw.empty() && !cin) {
                cin.clear();
                return lo; // safe fallback on EOF
            }
            try {
                size_t pos;
                int v = stoi(raw, &pos);
                // Reject if there are trailing non-whitespace chars (e.g. "1abc")
                string rest = raw.substr(pos);
                bool clean = true;
                for (char ch : rest) if (!isspace((unsigned char)ch)) { clean = false; break; }
                if (clean && v >= lo && v <= hi) return v;
                cout << "  [!] Enter a number between " << lo << " and " << hi << "." << endl;
            } catch (...) {
                cout << "  [!] Invalid number. Please try again." << endl;
            }
            if (++attempts > 20) return lo; // safety valve — never hang forever
        }
    }

    inline double promptDouble(const string& msg, double lo = 0.0, double hi = 1e9) {
        int attempts = 0;
        while (true) {
            string raw = prompt(msg);
            if (raw.empty() && !cin) { cin.clear(); return lo; }
            try {
                double v = stod(raw);
                if (v >= lo && v <= hi) return v;
                cout << "  [!] Enter a value between " << lo << " and " << hi << "." << endl;
            } catch (...) {
                cout << "  [!] Invalid number. Please try again." << endl;
            }
            if (++attempts > 20) return lo;
        }
    }

    inline bool promptYN(const string& msg) {
        int attempts = 0;
        while (true) {
            string raw = prompt(msg + " (y/n)");
            if (raw == "y" || raw == "Y") return true;
            if (raw == "n" || raw == "N") return false;
            if (!cin) { cin.clear(); return false; }
            cout << "  [!] Please enter 'y' or 'n'." << endl;
            if (++attempts > 10) return false;
        }
    }

    inline void success(const string& msg) { cout << "  [\xE2\x9C\x93] " << msg << endl; }
    inline void error(const string& msg)   { cout << "  [X] " << msg << endl; }
    inline void info(const string& msg)    { cout << "  [i] " << msg << endl; }

    inline void pause() {
        cout << "\n  Press Enter to continue...";
        // Flush output first, then wait for a newline
        cout.flush();
        string dummy;
        getline(cin, dummy);
    }

    inline void cls() {
#ifdef _WIN32
        system("cls");
#else
        system("clear");
#endif
    }
}
