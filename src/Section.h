#pragma once
#include <string>
#include <vector>
#include <iostream>
using namespace std;

class Section {
private:
    string sectionID;
    string courseID;
    string teacherID;
    string venueID;
    string timeSlot;        
    int    studentCount;      

    // For exam scheduling (multiple venues possible)
    vector<string> examVenueIDs;
    string         examTimeSlot;

public:
    Section() : studentCount(0) {}
    Section(const string& sid, const string& cid,
            const string& tid, const string& vid, const string& ts)
        : sectionID(sid), courseID(cid), teacherID(tid),
          venueID(vid), timeSlot(ts), studentCount(0) {}

    // Getters
    string getSectionID()   const { return sectionID; }
    string getCourseID()    const { return courseID; }
    string getTeacherID()   const { return teacherID; }
    string getVenueID()     const { return venueID; }
    string getTimeSlot()    const { return timeSlot; }
    int    getStudentCount()const { return studentCount; }
    string getExamTimeSlot()const { return examTimeSlot; }
    const vector<string>& getExamVenueIDs() const { return examVenueIDs; }

    // Setters
    void setSectionID(const string& s)  { sectionID = s; }
    void setCourseID(const string& s)   { courseID = s; }
    void setTeacherID(const string& s)  { teacherID = s; }
    void setVenueID(const string& s)    { venueID = s; }
    void setTimeSlot(const string& s)   { timeSlot = s; }
    void setStudentCount(int n)         { studentCount = n; }
    void setExamTimeSlot(const string& s) { examTimeSlot = s; }
    void addExamVenue(const string& vid) { examVenueIDs.push_back(vid); }
    void setExamVenues(const vector<string>& v) { examVenueIDs = v; }

    // Conflict check: two sections conflict if same time slot (for student)
    bool conflictsWith(const Section& other) const {
        return (timeSlot == other.timeSlot && timeSlot != "");
    }

    void display() const {
        cout << "  Section: " << sectionID
             << " | Course: " << courseID
             << " | Teacher: " << teacherID
             << " | Venue: " << venueID
             << " | Time: " << timeSlot
             << " | Students: " << studentCount << endl;
        if (!examTimeSlot.empty()) {
            cout << "    Exam Slot: " << examTimeSlot << " | Exam Venues: ";
            for (const auto& v : examVenueIDs) cout << v << " ";
            cout << endl;
        }
    }
};
