#pragma once
#include "AcademicEntity.h"
#include "Assessment.h"
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <iomanip>
using namespace std;

struct TranscriptEntry {
    string courseID;
    string courseTitle;
    double finalGrade;
    string letterGrade;
    string gradeStatus;
};

class Student : public AcademicEntity {
protected:
    string studentType;
    string statusFlag;
    vector<string> enrolledCourseIDs;
    map<string, double> courseFinalGrades;
  
    map<string, vector<Assessment*>> studentSectionAssessments;

    static string toLetterGrade(double pct) {
        if (pct >= 90) return "A+";
        if (pct >= 85) return "A";
        if (pct >= 80) return "A-";
        if (pct >= 75) return "B+";
        if (pct >= 70) return "B";
        if (pct >= 65) return "B-";
        if (pct >= 60) return "C+";
        if (pct >= 55) return "C";
        if (pct >= 50) return "C-";
        if (pct >= 45) return "D+";
        if (pct >= 40) return "D";
        return "F";
    }

    static double gradePoints(const string& lg) {
        if (lg == "A+") return 4.0;
        if (lg == "A")  return 4.0;
        if (lg == "A-") return 3.7;
        if (lg == "B+") return 3.3;
        if (lg == "B")  return 3.0;
        if (lg == "B-") return 2.7;
        if (lg == "C+") return 2.3;
        if (lg == "C")  return 2.0;
        if (lg == "C-") return 1.7;
        if (lg == "D+") return 1.3;
        if (lg == "D")  return 1.0;
        return 0.0;
    }

public:
    Student() : statusFlag("Active") {}
    Student(const string& id, const string& n, const string& e, const string& type)
        : AcademicEntity(id, n, e), studentType(type), statusFlag("Active") {}

    virtual ~Student() {
        for (auto& pair : studentSectionAssessments)
            for (auto a : pair.second) delete a;
    }

    virtual void calculateGPA() = 0;
    virtual void viewTranscript() const = 0;
    virtual string getStudentType() const = 0;

    bool isEnrolledIn(const string& cid) const {
        for (const auto& c : enrolledCourseIDs)
            if (c == cid) return true;
        return false;
    }
    void enrollCourse(const string& cid) {
        if (!isEnrolledIn(cid)) enrolledCourseIDs.push_back(cid);
    }
    void dropCourse(const string& cid) {
        for (auto it = enrolledCourseIDs.begin(); it != enrolledCourseIDs.end(); ++it) {
            if (*it == cid) { enrolledCourseIDs.erase(it); return; }
        }
    }

    
    void appendStudentAssessmentForSection(const string& secID, Assessment* a) {
        studentSectionAssessments[secID].push_back(a);
    }

   
    void initStudentAssessmentsForSection(const string& secID,
                                          const vector<Assessment*>& templateAssessments) {
        if (studentSectionAssessments.count(secID)) return; // already initialised
        for (auto a : templateAssessments) {
            Assessment* copy = nullptr;
            const string& t = a->getType();
            if      (t == "Exam")  copy = new Exam(0, a->getMaxScore(), a->getWeightage());
            else if (t == "Quiz")  copy = new Quiz(0, a->getMaxScore(), a->getWeightage());
            else                   copy = new Assignment(0, a->getMaxScore(), a->getWeightage());
            studentSectionAssessments[secID].push_back(copy);
        }
    }

    vector<Assessment*>& getStudentAssessmentsForSection(const string& secID) {
        return studentSectionAssessments[secID];
    }

    bool hasStudentAssessmentsForSection(const string& secID) const {
        auto it = studentSectionAssessments.find(secID);
        return it != studentSectionAssessments.end() && !it->second.empty();
    }

    const map<string, vector<Assessment*>>& getAllStudentSectionAssessments() const {
        return studentSectionAssessments;
    }

    void setFinalGrade(const string& cid, double grade) { courseFinalGrades[cid] = grade; }
    double getFinalGrade(const string& cid) const {
        auto it = courseFinalGrades.find(cid);
        return (it != courseFinalGrades.end()) ? it->second : -1;
    }
    const map<string, double>& getAllFinalGrades() const { return courseFinalGrades; }

    string getStudentStatus()  const { return statusFlag; }
    const vector<string>& getEnrolledCourseIDs() const { return enrolledCourseIDs; }
    void setStatusFlag(const string& s)           { statusFlag = s; }
    void setEnrolledCourses(const vector<string>& v) { enrolledCourseIDs = v; }
    void setFinalGrades(const map<string, double>& m) { courseFinalGrades = m; }
};


class RegularStudent : public Student {
    double gpa;
public:
    RegularStudent() : gpa(0.0) { studentType = "Regular"; }
    RegularStudent(const string& id, const string& n, const string& e)
        : Student(id, n, e, "Regular"), gpa(0.0) {}

    string getStudentType() const override { return "Regular"; }
    double getGPA() const { return gpa; }
    void   setGPA(double g) { gpa = g; }

    void calculateGPA() override {
        if (courseFinalGrades.empty()) { gpa = 0; return; }
        double total = 0; int count = 0;
        for (const auto& p : courseFinalGrades) {
            if (p.second >= 0) { total += gradePoints(toLetterGrade(p.second)); count++; }
        }
        gpa = (count > 0) ? total / count : 0;
    }

    void displayProfile() const override {
       
        cout << "  REGULAR STUDENT PROFILE" << endl;
        cout << "  ID    : " << ID << endl;
        cout << "  Name  : " << name << endl;
        cout << "  Email : " << email << endl;
        cout << "  GPA   : " << fixed << setprecision(2) << gpa << endl;
        cout << "  Status: " << statusFlag << endl;
        
    }

    void viewTranscript() const override {
        cout << "\n── Transcript: " << name << " (" << ID << ") ──" << endl;
        if (courseFinalGrades.empty()) { cout << "  No grades recorded." << endl; }
        else {
            cout << left << setw(12) << "CourseID" << setw(12) << "Grade(%)" << "Letter" << endl;
            cout << string(36, '-') << endl;
            for (const auto& p : courseFinalGrades) {
                string lg = toLetterGrade(p.second);
                cout << left << setw(12) << p.first
                     << setw(12) << fixed << setprecision(1) << p.second << lg << endl;
            }
        }
        cout << "  Cumulative GPA: " << fixed << setprecision(2) << gpa << endl;
    }
};

class ScholarshipStudent : public Student {
    double gpa;
    double minGPA;
public:
    ScholarshipStudent() : gpa(0.0), minGPA(2.5) { studentType = "Scholarship"; }
    ScholarshipStudent(const string& id, const string& n, const string& e, double min = 2.5)
        : Student(id, n, e, "Scholarship"), gpa(0.0), minGPA(min) {}

    string getStudentType() const override { return "Scholarship"; }
    double getGPA()    const { return gpa; }
    double getMinGPA() const { return minGPA; }
    void   setGPA(double g) { gpa = g; }
    void   setMinGPA(double m) { minGPA = m; }

    void calculateGPA() override {
        if (courseFinalGrades.empty()) { gpa = 0; statusFlag = "Active"; return; }
        double total = 0; int count = 0;
        for (const auto& p : courseFinalGrades) {
            if (p.second >= 0) { total += gradePoints(toLetterGrade(p.second)); count++; }
        }
        gpa = (count > 0) ? total / count : 0;
        statusFlag = (gpa < minGPA) ? "Probation" : "Active";
    }

    void displayProfile() const override {
        
        cout << "  SCHOLARSHIP STUDENT PROFILE" << endl;
        cout << "  ID     : " << ID << endl;
        cout << "  Name   : " << name << endl;
        cout << "  Email  : " << email << endl;
        cout << "  GPA    : " << fixed << setprecision(2) << gpa
             << " (Min required: " << minGPA << ")" << endl;
        cout << "  Status : " << statusFlag << endl;
       
    }

    void viewTranscript() const override {
        cout << "\n── Scholarship Transcript: " << name << " (" << ID << ") ──" << endl;
        if (courseFinalGrades.empty()) { cout << "  No grades recorded." << endl; }
        else {
            cout << left << setw(12) << "CourseID" << setw(12) << "Grade(%)" << "Letter" << endl;
            cout << string(36, '-') << endl;
            for (const auto& p : courseFinalGrades) {
                string lg = toLetterGrade(p.second);
                cout << left << setw(12) << p.first
                     << setw(12) << fixed << setprecision(1) << p.second << lg << endl;
            }
        }
        cout << "  GPA: " << fixed << setprecision(2) << gpa
             << " | Min Required: " << minGPA << " | Status: " << statusFlag << endl;
    }
};

class ExchangeStudent : public Student {
public:
    ExchangeStudent() { studentType = "Exchange"; }
    ExchangeStudent(const string& id, const string& n, const string& e)
        : Student(id, n, e, "Exchange") {}

    string getStudentType() const override { return "Exchange"; }
    void calculateGPA() override {} // No numeric GPA for exchange students

    void displayProfile() const override {
      
        cout << "  EXCHANGE STUDENT PROFILE" << endl;
        cout << "  ID     : " << ID << endl;
        cout << "  Name   : " << name << endl;
        cout << "  Email  : " << email << endl;
        cout << "  Grading: Pass/Fail only" << endl;
        cout << "  Status : " << statusFlag << endl;
      
    }

    void viewTranscript() const override {
        cout << "\n── Exchange Student Transcript: " << name << " (" << ID << ") ──" << endl;
        if (courseFinalGrades.empty()) { cout << "  No grades recorded." << endl; }
        else {
            cout << left << setw(12) << "CourseID" << setw(12) << "Grade(%)" << "Result" << endl;
            cout << string(36, '-') << endl;
            for (const auto& p : courseFinalGrades) {
                string result = (p.second >= 50) ? "PASS" : "FAIL";
                cout << left << setw(12) << p.first
                     << setw(12) << fixed << setprecision(1) << p.second << result << endl;
            }
        }
        cout << "  (Exchange students receive Pass/Fail credits only)" << endl;
    }
};
