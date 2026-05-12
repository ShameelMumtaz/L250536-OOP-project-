#pragma once
#include "AcademicEntity.h"
#include <string>
#include <vector>
#include <iostream>
#include <iomanip>
using namespace std;

// ============================================================
//  FeedbackEntry — one student rating + comment for a teacher
// ============================================================
struct FeedbackEntry {
    string studentID;
    int    rating;    // 1–5
    string comment;
};

// ============================================================
//  Teacher
// ============================================================
class Teacher : public AcademicEntity {
private:
    vector<string>        assignedCourseIDs;
    vector<FeedbackEntry> feedbackList;
    double                averageFeedback;

public:
    Teacher() : averageFeedback(0.0) {}
    Teacher(const string& id, const string& n, const string& e)
        : AcademicEntity(id, n, e), averageFeedback(0.0) {}

    // ── Feedback ──────────────────────────────────────────
    void addFeedback(const string& studentID, int rating, const string& comment) {
        if (rating < 1 || rating > 5) {
            cout << "  [!] Rating must be between 1 and 5." << endl;
            return;
        }
        feedbackList.push_back({studentID, rating, comment});
        recalculateAverage();
    }

    void recalculateAverage() {
        if (feedbackList.empty()) { averageFeedback = 0; return; }
        double sum = 0;
        for (const auto& f : feedbackList) sum += f.rating;
        averageFeedback = sum / feedbackList.size();
    }

    double getAverageFeedback() const { return averageFeedback; }
    void   setAverageFeedback(double v) { averageFeedback = v; }

    const vector<FeedbackEntry>& getFeedbackList() const { return feedbackList; }

    // ── Course assignments ────────────────────────────────
    void assignCourse(const string& cid) {
        for (const auto& c : assignedCourseIDs)
            if (c == cid) return;
        assignedCourseIDs.push_back(cid);
    }
    const vector<string>& getAssignedCourseIDs() const { return assignedCourseIDs; }
    void setAssignedCourses(const vector<string>& v) { assignedCourseIDs = v; }

    // ── Display ───────────────────────────────────────────
    void displayProfile() const override {
        cout << "\n╔══════════════════════════════════════╗" << endl;
        cout << "  TEACHER PROFILE" << endl;
        cout << "  ID      : " << ID << endl;
        cout << "  Name    : " << name << endl;
        cout << "  Email   : " << email << endl;
        cout << "  Score   : " << fixed << setprecision(2) << averageFeedback << "/5.0" << endl;
        cout << "  Courses : ";
        for (const auto& c : assignedCourseIDs) cout << c << " ";
        cout << endl;
        cout << "╚══════════════════════════════════════╝" << endl;
    }

    void displayFeedback() const {
        cout << "\n  Feedback for " << name << " (avg: "
             << fixed << setprecision(2) << averageFeedback << "/5):" << endl;
        if (feedbackList.empty()) {
            cout << "  No feedback yet." << endl;
            return;
        }
        int i = 1;
        for (const auto& f : feedbackList) {
            cout << "  " << i++ << ". [Student " << f.studentID << "] "
                 << "Rating: " << f.rating << "/5 — " << f.comment << endl;
        }
    }
};
