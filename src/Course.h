#pragma once
#include "Assessment.h"
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
using namespace std;

// ============================================================
//  Weightage config read from weightages.txt
//  Format: CourseType | Exam% | Assignment% | Quiz%
// ============================================================
struct WeightageConfig {
    double examPct   = 50;
    double assignPct = 30;
    double quizPct   = 20;
};

// ============================================================
//  Course — Abstract base
// ============================================================
class Course {
protected:
    string courseID;
    string title;
    string teacherID;
    string courseType;          // "Core", "Elective", "Lab"
    vector<string> enrolledStudentIDs;
    vector<Assessment*> assessments;
    WeightageConfig wConfig;

public:
    Course() {}
    Course(const string& cid, const string& t, const string& tid, const string& ct)
        : courseID(cid), title(t), teacherID(tid), courseType(ct) {}

    virtual ~Course() {
        for (auto a : assessments) delete a;
    }

    // Pure virtuals
    virtual double calculateFinalGrade(const vector<Assessment*>& asses) const = 0;
    virtual int    getExamDurationMinutes() const = 0;
    virtual bool   requiresComputers() const { return false; }

    // Enrollment helpers
    bool isEnrolled(const string& sID) const {
        for (const auto& s : enrolledStudentIDs)
            if (s == sID) return true;
        return false;
    }
    void enrollStudent(const string& sID) {
        if (!isEnrolled(sID)) enrolledStudentIDs.push_back(sID);
    }
    void removeStudent(const string& sID) {
        for (auto it = enrolledStudentIDs.begin(); it != enrolledStudentIDs.end(); ++it) {
            if (*it == sID) { enrolledStudentIDs.erase(it); return; }
        }
    }
    int getEnrollmentCount() const { return (int)enrolledStudentIDs.size(); }

    // Assessment management
    void addAssessment(Assessment* a) { assessments.push_back(a); }
    vector<Assessment*>& getAssessments() { return assessments; }
    const vector<Assessment*>& getAssessments() const { return assessments; }

    // Getters / Setters
    string getCourseID()   const { return courseID; }
    string getTitle()      const { return title; }
    string getTeacherID()  const { return teacherID; }
    string getCourseType() const { return courseType; }
    const vector<string>& getEnrolledStudentIDs() const { return enrolledStudentIDs; }
    WeightageConfig getWeightageConfig() const { return wConfig; }

    void setCourseID(const string& id) { courseID = id; }
    void setTitle(const string& t)     { title = t; }
    void setTeacherID(const string& t) { teacherID = t; }
    void setWeightageConfig(const WeightageConfig& w) { wConfig = w; }
    void setEnrolledStudents(const vector<string>& sv) { enrolledStudentIDs = sv; }

    virtual void displayInfo() const {
        cout << "  Course: [" << courseID << "] " << title
             << " | Type: " << courseType
             << " | Teacher: " << teacherID
             << " | Enrolled: " << enrolledStudentIDs.size() << endl;
    }
};

// ============================================================
//  CoreCourse — heavy final exam (3-hour slot)
// ============================================================
class CoreCourse : public Course {
public:
    CoreCourse() { courseType = "Core"; }
    CoreCourse(const string& cid, const string& t, const string& tid)
        : Course(cid, t, tid, "Core") {}

    double calculateFinalGrade(const vector<Assessment*>& asses) const override {
        double total = 0;
        for (auto a : asses) {
            total += a->getWeightedScore();
        }
        return total;
    }

    int getExamDurationMinutes() const override { return 180; } // 3 hours

    void displayInfo() const override {
        cout << "  [CORE COURSE] ";
        Course::displayInfo();
        cout << "    Exam Duration: 3 hours | Exam Wt: " << wConfig.examPct
             << "% | Assign Wt: " << wConfig.assignPct
             << "% | Quiz Wt: " << wConfig.quizPct << "%" << endl;
    }
};

// ============================================================
//  ElectiveCourse — mix of assignments and final project (2-hour slot)
// ============================================================
class ElectiveCourse : public Course {
public:
    ElectiveCourse() { courseType = "Elective"; }
    ElectiveCourse(const string& cid, const string& t, const string& tid)
        : Course(cid, t, tid, "Elective") {}

    double calculateFinalGrade(const vector<Assessment*>& asses) const override {
        double total = 0;
        for (auto a : asses) {
            total += a->getWeightedScore();
        }
        return total;
    }

    int getExamDurationMinutes() const override { return 120; } // 2 hours

    void displayInfo() const override {
        cout << "  [ELECTIVE COURSE] ";
        Course::displayInfo();
        cout << "    Exam Duration: 2 hours | Exam Wt: " << wConfig.examPct
             << "% | Assign Wt: " << wConfig.assignPct
             << "% | Quiz Wt: " << wConfig.quizPct << "%" << endl;
    }
};

// ============================================================
//  LabCourse — 100% continuous assessment, needs computers, no final exam
// ============================================================
class LabCourse : public Course {
public:
    LabCourse() { courseType = "Lab"; }
    LabCourse(const string& cid, const string& t, const string& tid)
        : Course(cid, t, tid, "Lab") {}

    double calculateFinalGrade(const vector<Assessment*>& asses) const override {
        // Lab: only Quiz and Assignment, no Exam
        double total = 0;
        for (auto a : asses) {
            if (a->getType() != "Exam")
                total += a->getWeightedScore();
        }
        return total;
    }

    int  getExamDurationMinutes() const override { return 0; } // No final exam
    bool requiresComputers()      const override { return true; }

    void displayInfo() const override {
        cout << "  [LAB COURSE] ";
        Course::displayInfo();
        cout << "    No Final Exam | Continuous Assessment | Requires Computers" << endl;
    }
};
