#pragma once
#include <string>
#include <iostream>
using namespace std;

// ============================================================
//  Assessment — Abstract base for all grading items
// ============================================================
class Assessment {
protected:
    string type;       // "Exam", "Quiz", "Assignment"
    double rawScore;
    double maxScore;
    double weightage;  // percentage weight in final grade (0–100)

public:
    Assessment() : rawScore(0), maxScore(100), weightage(0) {}
    Assessment(const string& t, double raw, double maxS, double w)
        : type(t), rawScore(raw), maxScore(maxS), weightage(w) {}

    virtual ~Assessment() {}

    // Each subclass can override how percentage is computed
    virtual double getPercentage() const {
        if (maxScore == 0) return 0;
        return (rawScore / maxScore) * 100.0;
    }

    // Contribution to final grade = percentage * weightage / 100
    double getWeightedScore() const {
        return getPercentage() * weightage / 100.0;
    }

    string getType()     const { return type; }
    double getRawScore() const { return rawScore; }
    double getMaxScore() const { return maxScore; }
    double getWeightage()const { return weightage; }

    void setRawScore(double s)  { rawScore = s; }
    void setMaxScore(double s)  { maxScore = s; }
    void setWeightage(double w) { weightage = w; }

    virtual void display() const {
        cout << "  [" << type << "] Score: " << rawScore << "/" << maxScore
             << "  Weightage: " << weightage << "%"
             << "  Contribution: " << getWeightedScore() << "%" << endl;
    }
};

// ============================================================
//  Exam
// ============================================================
class Exam : public Assessment {
public:
    Exam() : Assessment("Exam", 0, 100, 0) {}
    Exam(double raw, double maxS, double w) : Assessment("Exam", raw, maxS, w) {}

    void display() const override {
        cout << "  [EXAM] Score: " << rawScore << "/" << maxScore
             << "  Weightage: " << weightage << "%"
             << "  Contribution: " << getWeightedScore() << "%" << endl;
    }
};

// ============================================================
//  Quiz
// ============================================================
class Quiz : public Assessment {
public:
    Quiz() : Assessment("Quiz", 0, 100, 0) {}
    Quiz(double raw, double maxS, double w) : Assessment("Quiz", raw, maxS, w) {}

    void display() const override {
        cout << "  [QUIZ] Score: " << rawScore << "/" << maxScore
             << "  Weightage: " << weightage << "%"
             << "  Contribution: " << getWeightedScore() << "%" << endl;
    }
};


//  Assignment
// ============================================================
class Assignment : public Assessment {
public:
    Assignment() : Assessment("Assignment", 0, 100, 0) {}
    Assignment(double raw, double maxS, double w) : Assessment("Assignment", raw, maxS, w) {}

    void display() const override {
        cout << "  [ASSIGNMENT] Score: " << rawScore << "/" << maxScore
             << "  Weightage: " << weightage << "%"
             << "  Contribution: " << getWeightedScore() << "%" << endl;
    }
};
