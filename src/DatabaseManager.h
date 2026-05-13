#pragma once
#include "Student.h"
#include "Teacher.h"
#include "Course.h"
#include "Assessment.h"
#include "Venue.h"
#include "Section.h"
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
using namespace std;


class DatabaseManager {
private:
    string dataDir;

    static string trim(const string& s) {
        size_t a = s.find_first_not_of(" \t\r\n");
        size_t b = s.find_last_not_of(" \t\r\n");
        return (a == string::npos) ? "" : s.substr(a, b - a + 1);
    }

    static vector<string> split(const string& line, char delim = '|') {
        vector<string> parts;
        stringstream ss(line);
        string tok;
        while (getline(ss, tok, delim))
            parts.push_back(trim(tok));
        return parts;
    }

public:
    DatabaseManager(const string& dir = "data/") : dataDir(dir) {}

    map<string, WeightageConfig> loadWeightages() const {
        map<string, WeightageConfig> wmap;
        // Sensible defaults so the system works even without the file
        wmap["Core"]     = {50, 30, 20};
        wmap["Elective"] = {30, 40, 30};
        wmap["Lab"]      = {0,  60, 40};

        ifstream f(dataDir + "weightages.txt");
        if (!f.is_open()) {
            cerr << "  [!] weightages.txt not found — using defaults.\n";
            return wmap;
        }
        string line;
        while (getline(f, line)) {
            if (line.empty() || line[0] == '#') continue;
            auto p = split(line);
            if (p.size() < 4) continue;
            WeightageConfig w;
            try {
                w.examPct   = stod(p[1]);
                w.assignPct = stod(p[2]);
                w.quizPct   = stod(p[3]);
                wmap[p[0]]  = w;
            } catch (...) {}
        }
        return wmap;
    }

    void saveWeightages(const map<string, WeightageConfig>& wmap) const {
        ofstream f(dataDir + "weightages.txt");
        f << "# CourseType | Exam% | Assignment% | Quiz%\n";
        for (const auto& kv : wmap)
            f << kv.first << " | " << kv.second.examPct << " | "
              << kv.second.assignPct << " | " << kv.second.quizPct << "\n";
    }


    vector<Venue*> loadVenues() const {
        vector<Venue*> venues;
        ifstream f(dataDir + "Venues.txt");
        string line;
        while (getline(f, line)) {
            if (line.empty() || line[0] == '#') continue;
            auto p = split(line);
            if (p.size() < 3) continue;
            try { venues.push_back(new Venue(p[0], stoi(p[1]), p[2] == "1")); }
            catch (...) {}
        }
        return venues;
    }

    void saveVenues(const vector<Venue*>& venues) const {
        ofstream f(dataDir + "Venues.txt");
        f << "# RoomID | Capacity | HasComputers(1/0)\n";
        for (auto v : venues)
            f << v->getRoomID() << " | " << v->getCapacity()
              << " | " << (v->getHasComputers() ? 1 : 0) << "\n";
    }

 
    vector<Teacher*> loadTeachers() const {
        vector<Teacher*> teachers;
        ifstream f(dataDir + "Teachers.txt");
        string line;
        while (getline(f, line)) {
            if (line.empty() || line[0] == '#') continue;
            auto p = split(line);
            if (p.size() < 3) continue;
            Teacher* t = new Teacher(p[0], p[1], p[2]);
            if (p.size() >= 4) { try { t->setAverageFeedback(stod(p[3])); } catch (...) {} }
            if (p.size() >= 5 && !p[4].empty()) {
                vector<string> courses;
                stringstream ss(p[4]); string cid;
                while (getline(ss, cid, ',')) { string tc = trim(cid); if (!tc.empty()) courses.push_back(tc); }
                t->setAssignedCourses(courses);
            }
            teachers.push_back(t);
        }
        return teachers;
    }

    void saveTeachers(const vector<Teacher*>& teachers) const {
        ofstream f(dataDir + "Teachers.txt");
        f << "# ID | Name | Email | AvgFeedback | CourseIDs(comma-sep)\n";
        for (auto t : teachers) {
            f << t->getID() << " | " << t->getName() << " | "
              << t->getEmail() << " | " << t->getAverageFeedback() << " | ";
            const auto& cids = t->getAssignedCourseIDs();
            for (size_t i = 0; i < cids.size(); i++) {
                f << cids[i]; if (i + 1 < cids.size()) f << ",";
            }
            f << "\n";
        }
    }

  
    void saveFeedback(const vector<Teacher*>& teachers) const {
        ofstream f(dataDir + "Feedback.txt");
        f << "# TeacherID | StudentID | Rating | Comment\n";
        for (auto t : teachers) {
            for (const auto& fb : t->getFeedbackList()) {
                // Sanitise comment — replace | with ; so parsing stays unambiguous
                string safe = fb.comment;
                for (char& ch : safe) if (ch == '|') ch = ';';
                f << t->getID() << " | " << fb.studentID << " | "
                  << fb.rating << " | " << safe << "\n";
            }
        }
    }

    void loadFeedback(vector<Teacher*>& teachers) const {
        ifstream f(dataDir + "Feedback.txt");
        if (!f.is_open()) return; // first run — OK

        // Build quick lookup
        map<string, Teacher*> tmap;
        for (auto t : teachers) tmap[t->getID()] = t;

        string line;
        while (getline(f, line)) {
            if (line.empty() || line[0] == '#') continue;
            // Split only on first 3 pipes; rest is comment text
            size_t p1 = line.find('|');
            if (p1 == string::npos) continue;
            size_t p2 = line.find('|', p1 + 1);
            if (p2 == string::npos) continue;
            size_t p3 = line.find('|', p2 + 1);
            if (p3 == string::npos) continue;

            string tid     = trim(line.substr(0, p1));
            string sid     = trim(line.substr(p1+1, p2-p1-1));
            string ratingS = trim(line.substr(p2+1, p3-p2-1));
            string comment = trim(line.substr(p3 + 1));

            auto it = tmap.find(tid);
            if (it == tmap.end()) continue;
            try {
                int rating = stoi(ratingS);
                it->second->addFeedback(sid, rating, comment);
            } catch (...) {}
        }
    }

  
    vector<Student*> loadStudents() const {
        vector<Student*> students;
        ifstream f(dataDir + "Students.txt");
        string line;
        while (getline(f, line)) {
            if (line.empty() || line[0] == '#') continue;
            auto p = split(line);
            if (p.size() < 4) continue;

            string id    = p[0];
            string nm    = p[1];
            string em    = p[2];
            string type  = p[3];
            string extra = (p.size() > 4) ? p[4] : "0";
            string status= (p.size() > 5) ? p[5] : "Active";

            Student* s = nullptr;
            try {
                if (type == "Regular") {
                    RegularStudent* rs = new RegularStudent(id, nm, em);
                    rs->setGPA(extra == "N/A" ? 0.0 : stod(extra));
                    rs->setStatusFlag(status);
                    s = rs;
                } else if (type == "Scholarship") {
                    double gpa = 0, minG = 2.5;
                    auto gp = split(extra, ':');
                    if (gp.size() >= 2) { gpa = stod(gp[0]); minG = stod(gp[1]); }
                    else if (extra != "N/A") gpa = stod(extra);
                    ScholarshipStudent* ss2 = new ScholarshipStudent(id, nm, em, minG);
                    ss2->setGPA(gpa);
                    ss2->setStatusFlag(status);
                    s = ss2;
                } else if (type == "Exchange") {
                    ExchangeStudent* es = new ExchangeStudent(id, nm, em);
                    es->setStatusFlag(status);
                    s = es;
                } else continue;
            } catch (...) { continue; }

            // Enrolled course IDs at index 6
            if (p.size() > 6 && !p[6].empty()) {
                vector<string> cids;
                stringstream ss(p[6]); string cid;
                while (getline(ss, cid, ',')) { string tc = trim(cid); if (!tc.empty()) cids.push_back(tc); }
                s->setEnrolledCourses(cids);
            }
            // Final grades at index 7: courseID=grade,...
            if (p.size() > 7 && !p[7].empty()) {
                map<string, double> grades;
                stringstream ss(p[7]); string entry;
                while (getline(ss, entry, ',')) {
                    auto eq = entry.find('=');
                    if (eq != string::npos) {
                        try { grades[trim(entry.substr(0, eq))] = stod(trim(entry.substr(eq + 1))); }
                        catch (...) {}
                    }
                }
                s->setFinalGrades(grades);
            }
            students.push_back(s);
        }
        return students;
    }

    void saveStudents(const vector<Student*>& students) const {
        ofstream f(dataDir + "Students.txt");
        f << "# ID | Name | Email | Type | GPA/minGPA | Status | CourseIDs | Grades\n";
        for (auto s : students) {
            f << s->getID() << " | " << s->getName() << " | " << s->getEmail() << " | ";
            string type = s->getStudentType();
            f << type << " | ";

            if (type == "Regular") {
                RegularStudent* rs = dynamic_cast<RegularStudent*>(s);
                f << fixed << (rs ? rs->getGPA() : 0.0);
            } else if (type == "Scholarship") {
                ScholarshipStudent* ss2 = dynamic_cast<ScholarshipStudent*>(s);
                f << fixed << (ss2 ? ss2->getGPA() : 0.0) << ":" << (ss2 ? ss2->getMinGPA() : 2.5);
            } else {
                f << "N/A";
            }
            f << " | " << s->getStudentStatus() << " | ";

            const auto& cids = s->getEnrolledCourseIDs();
            for (size_t i = 0; i < cids.size(); i++) { f << cids[i]; if (i + 1 < cids.size()) f << ","; }
            f << " | ";

            const auto& grades = s->getAllFinalGrades();
            bool first = true;
            for (const auto& g : grades) {
                if (!first) f << ",";
                f << g.first << "=" << fixed << g.second;
                first = false;
            }
            f << "\n";
        }
    }

    
    vector<Course*> loadCourses(const map<string, WeightageConfig>& wmap) const {
        vector<Course*> courses;
        ifstream f(dataDir + "Courses.txt");
        string line;
        while (getline(f, line)) {
            if (line.empty() || line[0] == '#') continue;
            auto p = split(line);
            if (p.size() < 4) continue;

            Course* c = nullptr;
            string type = p[3];
            if      (type == "Core")     c = new CoreCourse(p[0], p[1], p[2]);
            else if (type == "Elective") c = new ElectiveCourse(p[0], p[1], p[2]);
            else if (type == "Lab")      c = new LabCourse(p[0], p[1], p[2]);
            else continue;

            auto it = wmap.find(type);
            if (it != wmap.end()) c->setWeightageConfig(it->second);

            if (p.size() > 4 && !p[4].empty()) {
                vector<string> sids;
                stringstream ss(p[4]); string sid;
                while (getline(ss, sid, ',')) { string ts = trim(sid); if (!ts.empty()) sids.push_back(ts); }
                c->setEnrolledStudents(sids);
            }
            courses.push_back(c);
        }
        return courses;
    }

    void saveCourses(const vector<Course*>& courses) const {
        ofstream f(dataDir + "Courses.txt");
        f << "# CourseID | Title | TeacherID | Type | EnrolledStudentIDs\n";
        for (auto c : courses) {
            f << c->getCourseID() << " | " << c->getTitle() << " | "
              << c->getTeacherID() << " | " << c->getCourseType() << " | ";
            const auto& sids = c->getEnrolledStudentIDs();
            for (size_t i = 0; i < sids.size(); i++) { f << sids[i]; if (i + 1 < sids.size()) f << ","; }
            f << "\n";
        }
    }

   
    vector<Section*> loadSections() const {
        vector<Section*> sections;
        ifstream f(dataDir + "sections.txt");
        string line;
        while (getline(f, line)) {
            if (line.empty() || line[0] == '#') continue;
            auto p = split(line);
            if (p.size() < 5) continue;
            Section* sec = new Section(p[0], p[1], p[2], p[3], p[4]);
            if (p.size() > 5 && !p[5].empty()) { try { sec->setStudentCount(stoi(p[5])); } catch (...) {} }
            if (p.size() > 6 && !p[6].empty()) sec->setExamTimeSlot(p[6]);
            if (p.size() > 7 && !p[7].empty()) {
                vector<string> evs;
                stringstream ss(p[7]); string ev;
                while (getline(ss, ev, ',')) { string te = trim(ev); if (!te.empty()) evs.push_back(te); }
                sec->setExamVenues(evs);
            }
            sections.push_back(sec);
        }
        return sections;
    }

    void saveSections(const vector<Section*>& sections) const {
        ofstream f(dataDir + "sections.txt");
        f << "# SectionID | CourseID | TeacherID | VenueID | TimeSlot | StudentCount | ExamSlot | ExamVenues\n";
        for (auto sec : sections) {
            f << sec->getSectionID() << " | " << sec->getCourseID() << " | "
              << sec->getTeacherID() << " | " << sec->getVenueID()  << " | "
              << sec->getTimeSlot()  << " | " << sec->getStudentCount() << " | "
              << sec->getExamTimeSlot() << " | ";
            const auto& evs = sec->getExamVenueIDs();
            for (size_t i = 0; i < evs.size(); i++) { f << evs[i]; if (i + 1 < evs.size()) f << ","; }
            f << "\n";
        }
    }

    
    map<string, vector<Assessment*>> loadAssessments() const {
        map<string, vector<Assessment*>> amap;
        ifstream f(dataDir + "assessments.txt");
        string line;
        while (getline(f, line)) {
            if (line.empty() || line[0] == '#') continue;
            auto p = split(line);
            if (p.size() < 5) continue;
            try {
                string sid  = p[0];
                string type = p[1];
                double raw  = stod(p[2]);
                double maxS = stod(p[3]);
                double wt   = stod(p[4]);
                Assessment* a = nullptr;
                if      (type == "Exam")       a = new Exam(raw, maxS, wt);
                else if (type == "Quiz")       a = new Quiz(raw, maxS, wt);
                else if (type == "Assignment") a = new Assignment(raw, maxS, wt);
                else continue;
                amap[sid].push_back(a);
            } catch (...) {}
        }
        return amap;
    }

    void saveAssessments(const map<string, vector<Assessment*>>& amap) const {
        ofstream f(dataDir + "assessments.txt");
        f << "# SectionID | Type | RawScore | MaxScore | Weightage\n";
        for (const auto& kv : amap)
            for (auto a : kv.second)
                f << kv.first << " | " << a->getType() << " | "
                  << a->getRawScore() << " | " << a->getMaxScore() << " | "
                  << a->getWeightage() << "\n";
    }


    void saveStudentMarks(const vector<Student*>& students) const {
        ofstream f(dataDir + "StudentMarks.txt");
        f << "# StudentID | SectionID | Type | RawScore | MaxScore | Weightage\n";
        for (auto s : students) {
            for (const auto& secPair : s->getAllStudentSectionAssessments()) {
                const string& secID = secPair.first;
                for (auto a : secPair.second) {
                    f << s->getID()        << " | "
                      << secID             << " | "
                      << a->getType()      << " | "
                      << a->getRawScore()  << " | "
                      << a->getMaxScore()  << " | "
                      << a->getWeightage() << "\n";
                }
            }
        }
    }

    void loadStudentMarks(vector<Student*>& students) const {
        ifstream f(dataDir + "StudentMarks.txt");
        if (!f.is_open()) return; // OK on first run

        map<string, Student*> smap;
        for (auto s : students) smap[s->getID()] = s;

        string line;
        while (getline(f, line)) {
            if (line.empty() || line[0] == '#') continue;
            auto p = split(line);
            if (p.size() < 6) continue;
            auto it = smap.find(p[0]);
            if (it == smap.end()) continue;
            try {
                string type = p[2];
                double raw  = stod(p[3]);
                double maxS = stod(p[4]);
                double wt   = stod(p[5]);
                Assessment* a = nullptr;
                if      (type == "Exam")       a = new Exam(raw, maxS, wt);
                else if (type == "Quiz")       a = new Quiz(raw, maxS, wt);
                else if (type == "Assignment") a = new Assignment(raw, maxS, wt);
                else continue;
                it->second->appendStudentAssessmentForSection(p[1], a);
            } catch (...) {}
        }
    }


    void saveExamSchedule(const vector<Section*>& sections) const {
        ofstream f(dataDir + "exam_schedule.txt");
        f << "# === EXAM SCHEDULE ===\n";
        f << "# SectionID | CourseID | ExamSlot | Venues\n";
        for (auto sec : sections) {
            if (sec->getExamTimeSlot().empty() || sec->getExamTimeSlot() == "UNSCHEDULED") continue;
            f << sec->getSectionID() << " | " << sec->getCourseID() << " | "
              << sec->getExamTimeSlot() << " | ";
            const auto& evs = sec->getExamVenueIDs();
            for (size_t i = 0; i < evs.size(); i++) { f << evs[i]; if (i + 1 < evs.size()) f << ","; }
            f << "\n";
        }
    }

  
    void saveAll(const vector<Student*>& students,
                 const vector<Teacher*>& teachers,
                 const vector<Course*>&  courses,
                 const vector<Section*>& sections,
                 const vector<Venue*>&   venues,
                 const map<string, WeightageConfig>& weightages,
                 const map<string, vector<Assessment*>>& sectionAssessments) const {
        saveWeightages(weightages);
        saveVenues(venues);
        saveTeachers(teachers);
        saveFeedback(teachers);
        saveStudents(students);
        saveCourses(courses);
        saveSections(sections);
        saveAssessments(sectionAssessments);
        saveStudentMarks(students);
        saveExamSchedule(sections);
    }
};
