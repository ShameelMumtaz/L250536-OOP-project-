#pragma once
#include "Student.h"
#include "Teacher.h"
#include "Course.h"
#include "Assessment.h"
#include "Section.h"
#include "Venue.h"
#include "DatabaseManager.h"
#include "Scheduler.h"
#include "UI.h"
#include <vector>
#include <string>
#include <iostream>
#include <map>
#include <algorithm>
#include <iomanip>
using namespace std;

class AcademicSystem {
private:
    DatabaseManager  db;
    Scheduler        scheduler;

    vector<Student*> students;
    vector<Teacher*> teachers;
    vector<Course*>  courses;
    vector<Section*> sections;
    vector<Venue*>   venues;
    map<string, WeightageConfig>        weightages;
    map<string, vector<Assessment*>>    sectionAssessments; 

    
    Student* findStudent(const string& id) const {
        for (auto s : students) if (s->getID() == id) return s;
        return nullptr;
    }
    Teacher* findTeacher(const string& id) const {
        for (auto t : teachers) if (t->getID() == id) return t;
        return nullptr;
    }
    Course*  findCourse (const string& id) const {
        for (auto c : courses) if (c->getCourseID() == id) return c;
        return nullptr;
    }
    Section* findSection(const string& id) const {
        for (auto s : sections) if (s->getSectionID() == id) return s;
        return nullptr;
    }
    Venue*   findVenue  (const string& id) const {
        for (auto v : venues) if (v->getRoomID() == id) return v;
        return nullptr;
    }

    bool studentExists(const string& id) const { return findStudent(id) != nullptr; }
    bool teacherExists(const string& id) const { return findTeacher(id) != nullptr; }
    bool courseExists (const string& id) const { return findCourse(id)  != nullptr; }
    bool sectionExists(const string& id) const { return findSection(id) != nullptr; }
    bool venueExists  (const string& id) const { return findVenue(id)   != nullptr; }

    string nextID(const string& prefix, const vector<string>& ids) const {
        int maxNum = 999;
        for (const auto& id : ids) {
            if (id.size() > prefix.size() && id.substr(0, prefix.size()) == prefix) {
                try { int n = stoi(id.substr(prefix.size())); if (n > maxNum) maxNum = n; }
                catch (...) {}
            }
        }
        return prefix + to_string(maxNum + 1);
    }
    string nextStudentID() const {
        vector<string> ids; for (auto s : students) ids.push_back(s->getID());
        return nextID("S", ids);
    }
    string nextTeacherID() const {
        vector<string> ids; for (auto t : teachers) ids.push_back(t->getID());
        return nextID("T", ids);
    }
    string nextCourseID() const {
        vector<string> ids; for (auto c : courses) ids.push_back(c->getCourseID());
        return nextID("C", ids);
    }
    string nextSectionID() const {
        vector<string> ids; for (auto s : sections) ids.push_back(s->getSectionID());
        return nextID("SEC", ids);
    }
    string nextVenueID() const {
        vector<string> ids; for (auto v : venues) ids.push_back(v->getRoomID());
        return nextID("V", ids);
    }

public:
    AcademicSystem(const string& dataDir = "data/") : db(dataDir) {}

    ~AcademicSystem() {
        for (auto s : students) delete s;
        for (auto t : teachers) delete t;
        for (auto c : courses)  delete c;
        for (auto s : sections) delete s;
        for (auto v : venues)   delete v;
        for (auto& kv : sectionAssessments)
            for (auto a : kv.second) delete a;
    }

    void loadAll() {
        weightages         = db.loadWeightages();
        venues             = db.loadVenues();
        teachers           = db.loadTeachers();
        db.loadFeedback(teachers);          // ← restores individual feedback entries
        students           = db.loadStudents();
        courses            = db.loadCourses(weightages);
        sections           = db.loadSections();
        sectionAssessments = db.loadAssessments();
        db.loadStudentMarks(students);      // ← restores per-student marks
        UI::success("All data loaded from files.");
    }

    void saveAll() {
        db.saveAll(students, teachers, courses, sections, venues,
                   weightages, sectionAssessments);
        UI::success("All data saved to files.");
    }


    void menuStudents() {
        while (true) {
            UI::banner("STUDENT MANAGEMENT");
            cout << "  1. Add Student\n"
                 << "  2. View All Students\n"
                 << "  3. View Student Profile\n"
                 << "  4. Edit Student\n"
                 << "  5. Delete Student\n"
                 << "  6. View Transcript\n"
                 << "  0. Back\n";
            UI::divider();
            int ch = UI::promptInt("Choice", 0, 6);
            switch (ch) {
                case 1: addStudent();        break;
                case 2: listStudents();      break;
                case 3: viewStudentProfile();break;
                case 4: editStudent();       break;
                case 5: deleteStudent();     break;
                case 6: viewTranscript();    break;
                case 0: return;
            }
            UI::pause();
        }
    }

    void addStudent() {
        UI::banner("ADD STUDENT");
        cout << "  Types: 1=Regular  2=Scholarship  3=Exchange\n";
        int type = UI::promptInt("Student type", 1, 3);

        string id = nextStudentID();
        string nm = UI::prompt("Full Name");
        if (nm.empty()) { UI::error("Name cannot be empty."); return; }
        string em = UI::prompt("Email");

        Student* s = nullptr;
        if (type == 1) {
            s = new RegularStudent(id, nm, em);
        } else if (type == 2) {
            double minG = UI::promptDouble("Minimum GPA required (e.g. 2.5)", 0.0, 4.0);
            s = new ScholarshipStudent(id, nm, em, minG);
        } else {
            s = new ExchangeStudent(id, nm, em);
        }
        students.push_back(s);
        UI::success("Student added with ID: " + id);
    }

    void listStudents() {
        UI::banner("ALL STUDENTS");
        if (students.empty()) { UI::info("No students found."); return; }
        cout << left << setw(10) << "ID" << setw(25) << "Name"
             << setw(15) << "Type" << "Status" << "\n";
        UI::divider();
        for (auto s : students)
            cout << left << setw(10) << s->getID()
                 << setw(25) << s->getName()
                 << setw(15) << s->getStudentType()
                 << s->getStudentStatus() << "\n";
    }

    void viewStudentProfile() {
        string id = UI::prompt("Enter Student ID");
        Student* s = findStudent(id);
        if (!s) { UI::error("Student not found."); return; }
        s->displayProfile();
    }

    void editStudent() {
        string id = UI::prompt("Enter Student ID to edit");
        Student* s = findStudent(id);
        if (!s) { UI::error("Student not found."); return; }

        string nm = UI::prompt("New Name (leave blank to keep '" + s->getName() + "')");
        if (!nm.empty()) s->setName(nm);
        string em = UI::prompt("New Email (leave blank to keep '" + s->getEmail() + "')");
        if (!em.empty()) s->setEmail(em);
        UI::success("Student updated.");
    }

    void deleteStudent() {
        string id = UI::prompt("Enter Student ID to delete");
        for (auto it = students.begin(); it != students.end(); ++it) {
            if ((*it)->getID() == id) {
                Student* s = *it;
                // Remove from all courses + decrement section counts
                for (const auto& cid : s->getEnrolledCourseIDs()) {
                    Course* c = findCourse(cid);
                    if (c) c->removeStudent(id);
                    for (auto sec : sections)
                        if (sec->getCourseID() == cid)
                            sec->setStudentCount(max(0, sec->getStudentCount() - 1));
                }
                delete s;
                students.erase(it);
                UI::success("Student " + id + " deleted.");
                return;
            }
        }
        UI::error("Student not found.");
    }

    void viewTranscript() {
        string id = UI::prompt("Enter Student ID");
        Student* s = findStudent(id);
        if (!s) { UI::error("Student not found."); return; }
        s->calculateGPA();
        s->viewTranscript();
    }

    
    void menuTeachers() {
        while (true) {
            UI::banner("TEACHER MANAGEMENT");
            cout << "  1. Add Teacher\n"
                 << "  2. View All Teachers\n"
                 << "  3. View Teacher Profile\n"
                 << "  4. Edit Teacher\n"
                 << "  5. Delete Teacher\n"
                 << "  6. View Teacher Feedback\n"
                 << "  0. Back\n";
            UI::divider();
            int ch = UI::promptInt("Choice", 0, 6);
            switch (ch) {
                case 1: addTeacher();          break;
                case 2: listTeachers();        break;
                case 3: viewTeacherProfile();  break;
                case 4: editTeacher();         break;
                case 5: deleteTeacher();       break;
                case 6: viewTeacherFeedback(); break;
                case 0: return;
            }
            UI::pause();
        }
    }

    void addTeacher() {
        UI::banner("ADD TEACHER");
        string id = nextTeacherID();
        string nm = UI::prompt("Full Name");
        if (nm.empty()) { UI::error("Name cannot be empty."); return; }
        string em = UI::prompt("Email");
        teachers.push_back(new Teacher(id, nm, em));
        UI::success("Teacher added with ID: " + id);
    }

    void listTeachers() {
        UI::banner("ALL TEACHERS");
        if (teachers.empty()) { UI::info("No teachers found."); return; }
        cout << left << setw(10) << "ID" << setw(25) << "Name"
             << setw(32) << "Email" << "Avg Score\n";
        UI::divider();
        for (auto t : teachers)
            cout << left << setw(10) << t->getID()
                 << setw(25) << t->getName()
                 << setw(32) << t->getEmail()
                 << fixed << setprecision(2) << t->getAverageFeedback() << "/5\n";
    }

    void viewTeacherProfile() {
        string id = UI::prompt("Enter Teacher ID");
        Teacher* t = findTeacher(id);
        if (!t) { UI::error("Teacher not found."); return; }
        t->displayProfile();
    }

    void editTeacher() {
        string id = UI::prompt("Enter Teacher ID to edit");
        Teacher* t = findTeacher(id);
        if (!t) { UI::error("Teacher not found."); return; }
        string nm = UI::prompt("New Name (blank to keep '" + t->getName() + "')");
        if (!nm.empty()) t->setName(nm);
        string em = UI::prompt("New Email (blank to keep '" + t->getEmail() + "')");
        if (!em.empty()) t->setEmail(em);
        UI::success("Teacher updated.");
    }

    void deleteTeacher() {
        string id = UI::prompt("Enter Teacher ID to delete");
        for (auto it = teachers.begin(); it != teachers.end(); ++it) {
            if ((*it)->getID() == id) {
                // Warn about courses that still reference this teacher
                for (auto c : courses)
                    if (c->getTeacherID() == id)
                        UI::info("Warning: Course " + c->getCourseID() +
                                 " still references deleted teacher " + id + ".");
                delete *it;
                teachers.erase(it);
                UI::success("Teacher " + id + " deleted.");
                return;
            }
        }
        UI::error("Teacher not found.");
    }

    void viewTeacherFeedback() {
        string id = UI::prompt("Enter Teacher ID");
        Teacher* t = findTeacher(id);
        if (!t) { UI::error("Teacher not found."); return; }
        t->displayFeedback();
    }

   
    void menuCourses() {
        while (true) {
            UI::banner("COURSE MANAGEMENT");
            cout << "  1. Add Course\n"
                 << "  2. View All Courses\n"
                 << "  3. View Course Details\n"
                 << "  4. Edit Course Title\n"
                 << "  5. Delete Course\n"
                 << "  6. Configure Weightages\n"
                 << "  0. Back\n";
            UI::divider();
            int ch = UI::promptInt("Choice", 0, 6);
            switch (ch) {
                case 1: addCourse();         break;
                case 2: listCourses();       break;
                case 3: viewCourseDetails(); break;
                case 4: editCourse();        break;
                case 5: deleteCourse();      break;
                case 6: configWeightages();  break;
                case 0: return;
            }
            UI::pause();
        }
    }

    void addCourse() {
        UI::banner("ADD COURSE");
        cout << "  Types: 1=Core  2=Elective  3=Lab\n";
        int type = UI::promptInt("Course type", 1, 3);

        string id  = nextCourseID();
        string ttl = UI::prompt("Course Title");
        if (ttl.empty()) { UI::error("Title cannot be empty."); return; }
        string tid = UI::prompt("Teacher ID");
        if (!teacherExists(tid)) { UI::error("Teacher not found."); return; }

        Course* c = nullptr;
        string ct;
        if      (type == 1) { c = new CoreCourse(id, ttl, tid);     ct = "Core"; }
        else if (type == 2) { c = new ElectiveCourse(id, ttl, tid); ct = "Elective"; }
        else                { c = new LabCourse(id, ttl, tid);      ct = "Lab"; }

        auto it = weightages.find(ct);
        if (it != weightages.end()) c->setWeightageConfig(it->second);
        courses.push_back(c);

        Teacher* t = findTeacher(tid);
        if (t) t->assignCourse(id);

        UI::success("Course added with ID: " + id);
    }

    void listCourses() {
        UI::banner("ALL COURSES");
        if (courses.empty()) { UI::info("No courses found."); return; }
        cout << left << setw(10) << "ID" << setw(30) << "Title"
             << setw(12) << "Type" << setw(10) << "Teacher" << "Students\n";
        UI::divider();
        for (auto c : courses)
            cout << left << setw(10) << c->getCourseID()
                 << setw(30) << c->getTitle()
                 << setw(12) << c->getCourseType()
                 << setw(10) << c->getTeacherID()
                 << c->getEnrollmentCount() << "\n";
    }

    void viewCourseDetails() {
        string id = UI::prompt("Enter Course ID");
        Course* c = findCourse(id);
        if (!c) { UI::error("Course not found."); return; }
        c->displayInfo();
        cout << "  Enrolled Students: ";
        for (const auto& s : c->getEnrolledStudentIDs()) cout << s << " ";
        cout << "\n";
    }

    void editCourse() {
        string id = UI::prompt("Enter Course ID to edit");
        Course* c = findCourse(id);
        if (!c) { UI::error("Course not found."); return; }
        string ttl = UI::prompt("New Title (blank to keep '" + c->getTitle() + "')");
        if (!ttl.empty()) c->setTitle(ttl);
        UI::success("Course updated.");
    }

    void deleteCourse() {
        string id = UI::prompt("Enter Course ID to delete");
        for (auto it = courses.begin(); it != courses.end(); ++it) {
            if ((*it)->getCourseID() == id) {
                // Unenroll all students from this course
                for (auto s : students)
                    if (s->isEnrolledIn(id)) s->dropCourse(id);
                // Remove from teacher's list
                for (auto t : teachers) {
                    const auto& cids = t->getAssignedCourseIDs();
                    vector<string> updated;
                    for (const auto& c : cids) if (c != id) updated.push_back(c);
                    t->setAssignedCourses(updated);
                }
                // Remove associated sections
                sections.erase(
                    remove_if(sections.begin(), sections.end(),
                        [&](Section* sec) {
                            if (sec->getCourseID() == id) { delete sec; return true; }
                            return false;
                        }),
                    sections.end());
                delete *it;
                courses.erase(it);
                UI::success("Course " + id + " deleted (students unenrolled, sections removed).");
                return;
            }
        }
        UI::error("Course not found.");
    }

    void configWeightages() {
        UI::banner("CONFIGURE WEIGHTAGES");
        cout << "  Current weightages:\n";
        for (const auto& kv : weightages)
            cout << "  " << kv.first
                 << " → Exam: " << kv.second.examPct
                 << "%  Assign: " << kv.second.assignPct
                 << "%  Quiz: "   << kv.second.quizPct << "%\n";
        UI::divider();
        cout << "  Which type? (Core / Elective / Lab)\n";
        string type = UI::prompt("Type");
        if (type != "Core" && type != "Elective" && type != "Lab") {
            UI::error("Invalid type. Must be Core, Elective, or Lab."); return;
        }
        double exam   = UI::promptDouble("Exam %",       0, 100);
        double assign = UI::promptDouble("Assignment %", 0, 100);
        double quiz   = UI::promptDouble("Quiz %",       0, 100);
        if (exam + assign + quiz > 100.01) {
            UI::error("Weightages exceed 100%. Please re-enter."); return;
        }
        WeightageConfig& w = weightages[type];
        w.examPct = exam; w.assignPct = assign; w.quizPct = quiz;
        // Apply updated weightage to all courses of this type
        for (auto c : courses)
            if (c->getCourseType() == type) c->setWeightageConfig(w);
        db.saveWeightages(weightages);
        UI::success("Weightages for " + type + " updated and saved.");
    }


    void menuRegistration() {
        while (true) {
            UI::banner("SMART REGISTRATION");
            cout << "  1. Register Student in Section\n"
                 << "  2. Drop Student from Course\n"
                 << "  3. View Student's Enrolled Courses\n"
                 << "  0. Back\n";
            UI::divider();
            int ch = UI::promptInt("Choice", 0, 3);
            switch (ch) {
                case 1: registerStudent(); break;
                case 2: dropStudent();     break;
                case 3: viewEnrollments(); break;
                case 0: return;
            }
            UI::pause();
        }
    }

    void registerStudent() {
        UI::banner("REGISTER STUDENT");
        string sid = UI::prompt("Student ID");
        Student* s = findStudent(sid);
        if (!s) { UI::error("Student not found."); return; }

        string secID = UI::prompt("Section ID");
        Section* sec = findSection(secID);
        if (!sec) { UI::error("Section not found."); return; }

        Course* c = findCourse(sec->getCourseID());
        if (!c) { UI::error("Course for section not found."); return; }

        // 1. Duplicate enrollment check
        if (s->isEnrolledIn(c->getCourseID())) {
            UI::error("Student is already enrolled in course " + c->getCourseID() + ".");
            return;
        }

        // 2. Capacity check
        Venue* v = findVenue(sec->getVenueID());
        if (v && sec->getStudentCount() >= v->getCapacity()) {
            UI::error("Section is FULL (capacity: " + to_string(v->getCapacity()) + ").");
            return;
        }

        // 3. Time conflict check — student cannot be in two sections at the same time
        if (!sec->getTimeSlot().empty()) {
            for (const auto& enrolledCID : s->getEnrolledCourseIDs()) {
                for (auto existSec : sections) {
                    if (existSec->getSectionID() == secID) continue; // same section
                    if (existSec->getCourseID() == enrolledCID &&
                        existSec->getTimeSlot() == sec->getTimeSlot()) {
                        UI::error("TIME CONFLICT: Student already has section "
                                  + existSec->getSectionID() + " at " + sec->getTimeSlot() + ".");
                        return;
                    }
                }
            }
        }

        // All checks passed — enroll
        s->enrollCourse(c->getCourseID());
        c->enrollStudent(sid);
        sec->setStudentCount(sec->getStudentCount() + 1);

        // Initialise per-student assessment copies from template (if template exists)
        if (sectionAssessments.count(secID) && !sectionAssessments.at(secID).empty())
            s->initStudentAssessmentsForSection(secID, sectionAssessments.at(secID));

        UI::success("Student " + sid + " registered in section " + secID +
                    " (" + c->getTitle() + ").");
    }

    void dropStudent() {
        string sid = UI::prompt("Student ID");
        Student* s = findStudent(sid);
        if (!s) { UI::error("Student not found."); return; }

        string cid = UI::prompt("Course ID to drop");
        if (!s->isEnrolledIn(cid)) {
            UI::error("Student is not enrolled in course " + cid + "."); return;
        }
        s->dropCourse(cid);
        Course* c = findCourse(cid);
        if (c) c->removeStudent(sid);
        for (auto sec : sections) {
            if (sec->getCourseID() == cid)
                sec->setStudentCount(max(0, sec->getStudentCount() - 1));
        }
        UI::success("Student " + sid + " dropped from course " + cid + ".");
    }

    void viewEnrollments() {
        string sid = UI::prompt("Student ID");
        Student* s = findStudent(sid);
        if (!s) { UI::error("Student not found."); return; }
        cout << "\n  Enrolled courses for " << s->getName() << ":\n";
        const auto& cids = s->getEnrolledCourseIDs();
        if (cids.empty()) { UI::info("No courses enrolled."); return; }
        for (const auto& cid : cids) {
            Course* c = findCourse(cid);
            cout << "  → " << cid;
            if (c) cout << " — " << c->getTitle() << " (" << c->getCourseType() << ")";
            cout << "\n";
        }
    }

   
    void menuSections() {
        while (true) {
            UI::banner("SECTION MANAGEMENT");
            cout << "  1. Create Section\n"
                 << "  2. View All Sections\n"
                 << "  3. View Section Details\n"
                 << "  4. Delete Section\n"
                 << "  0. Back\n";
            UI::divider();
            int ch = UI::promptInt("Choice", 0, 4);
            switch (ch) {
                case 1: createSection();      break;
                case 2: listSections();       break;
                case 3: viewSectionDetails(); break;
                case 4: deleteSection();      break;
                case 0: return;
            }
            UI::pause();
        }
    }

    void createSection() {
        UI::banner("CREATE SECTION");
        string cid = UI::prompt("Course ID");
        if (!courseExists(cid)) { UI::error("Course not found."); return; }
        string tid = UI::prompt("Teacher ID");
        if (!teacherExists(tid)) { UI::error("Teacher not found."); return; }
        string ts  = UI::prompt("Time Slot (e.g. Mon 09:00-11:00)");

        // Auto-assign best venue
        Course* c     = findCourse(cid);
        bool needComp = c ? c->requiresComputers() : false;
        string vid    = "";
        for (auto v : venues) {
            if (needComp && !v->getHasComputers()) continue;
            vid = v->getRoomID();
            break;
        }
        if (vid.empty()) {
            vid = UI::prompt("No suitable venue found automatically. Enter Venue ID");
            if (!venueExists(vid)) { UI::error("Venue not found."); return; }
        }

        string secID = nextSectionID();
        sections.push_back(new Section(secID, cid, tid, vid, ts));
        UI::success("Section " + secID + " created (Course: " + cid + ", Venue: " + vid + ").");
    }

    void listSections() {
        UI::banner("ALL SECTIONS");
        if (sections.empty()) { UI::info("No sections found."); return; }
        cout << left << setw(10) << "SecID" << setw(10) << "CourseID"
             << setw(10) << "Teacher" << setw(8) << "Venue"
             << setw(22) << "Time Slot" << "Students\n";
        UI::divider();
        for (auto sec : sections)
            cout << left << setw(10) << sec->getSectionID()
                 << setw(10) << sec->getCourseID()
                 << setw(10) << sec->getTeacherID()
                 << setw(8)  << sec->getVenueID()
                 << setw(22) << sec->getTimeSlot()
                 << sec->getStudentCount() << "\n";
    }

    void viewSectionDetails() {
        string id = UI::prompt("Enter Section ID");
        Section* sec = findSection(id);
        if (!sec) { UI::error("Section not found."); return; }
        sec->display();
        // Also show assessment template if it exists
        if (sectionAssessments.count(id) && !sectionAssessments.at(id).empty()) {
            cout << "  Assessment Template:\n";
            for (auto a : sectionAssessments.at(id)) a->display();
        }
    }

    void deleteSection() {
        string id = UI::prompt("Enter Section ID to delete");
        for (auto it = sections.begin(); it != sections.end(); ++it) {
            if ((*it)->getSectionID() == id) {
                Section* sec = *it;
                string cid = sec->getCourseID();
                Course* c = findCourse(cid);
                if (c) {
                    for (const auto& enSID : c->getEnrolledStudentIDs()) {
                        Student* s = findStudent(enSID);
                        if (s) s->dropCourse(cid);
                    }
                    c->setEnrolledStudents({});
                }
                // Clean up assessment template
                for (auto a : sectionAssessments[id]) delete a;
                sectionAssessments.erase(id);
                delete sec;
                sections.erase(it);
                UI::success("Section " + id + " deleted.");
                return;
            }
        }
        UI::error("Section not found.");
    }

    void menuVenues() {
        while (true) {
            UI::banner("VENUE MANAGEMENT");
            cout << "  1. Add Venue\n"
                 << "  2. View All Venues\n"
                 << "  3. Delete Venue\n"
                 << "  0. Back\n";
            UI::divider();
            int ch = UI::promptInt("Choice", 0, 3);
            switch (ch) {
                case 1: addVenue();    break;
                case 2: listVenues();  break;
                case 3: deleteVenue(); break;
                case 0: return;
            }
            UI::pause();
        }
    }

    void addVenue() {
        string id = nextVenueID();
        int cap   = UI::promptInt("Capacity", 1, 2000);
        bool comp = UI::promptYN("Has computers?");
        venues.push_back(new Venue(id, cap, comp));
        UI::success("Venue " + id + " added.");
    }

    void listVenues() {
        UI::banner("ALL VENUES");
        if (venues.empty()) { UI::info("No venues found."); return; }
        cout << left << setw(10) << "RoomID" << setw(12) << "Capacity" << "Computers\n";
        UI::divider();
        for (auto v : venues)
            cout << left << setw(10) << v->getRoomID()
                 << setw(12) << v->getCapacity()
                 << (v->getHasComputers() ? "Yes" : "No") << "\n";
    }

    void deleteVenue() {
        string id = UI::prompt("Enter Venue ID to delete");
        for (auto it = venues.begin(); it != venues.end(); ++it) {
            if ((*it)->getRoomID() == id) {
                delete *it; venues.erase(it);
                UI::success("Venue " + id + " deleted.");
                return;
            }
        }
        UI::error("Venue not found.");
    }

    void menuFeedback() {
        while (true) {
            UI::banner("TEACHER FEEDBACK");
            cout << "  1. Submit Feedback for a Teacher\n"
                 << "  2. View All Feedback for a Teacher\n"
                 << "  0. Back\n";
            UI::divider();
            int ch = UI::promptInt("Choice", 0, 2);
            if (ch == 0) return;

            if (ch == 1) {
                string sid = UI::prompt("Your Student ID");
                Student* s = findStudent(sid);
                if (!s) { UI::error("Student not found."); UI::pause(); continue; }

                string tid = UI::prompt("Teacher ID to rate");
                Teacher* t = findTeacher(tid);
                if (!t) { UI::error("Teacher not found."); UI::pause(); continue; }

                // Verify student is actually in one of this teacher's courses
                bool related = false;
                for (const auto& cid : t->getAssignedCourseIDs())
                    if (s->isEnrolledIn(cid)) { related = true; break; }
                if (!related)
                    UI::info("Note: Student is not currently enrolled in any of this teacher's courses.");

                t->displayProfile();
                int    rating  = UI::promptInt("Your rating (1-5)", 1, 5);
                string comment = UI::prompt("Comment");
                t->addFeedback(sid, rating, comment);
                UI::success("Feedback submitted. New average: "
                            + to_string(t->getAverageFeedback()).substr(0, 4) + "/5.");
            } else {
                viewTeacherFeedback();
            }
            UI::pause();
        }
    }

    void menuGrading() {
        while (true) {
            UI::banner("GRADING SYSTEM");
            cout << "  1. Add/Define Assessment Template for Section\n"
                 << "  2. Enter Marks for a Student\n"
                 << "  3. View Section Assessment Template\n"
                 << "  4. Recalculate Grades for All Students in Section\n"
                 << "  5. View Student Marks in Section\n"
                 << "  0. Back\n";
            UI::divider();
            int ch = UI::promptInt("Choice", 0, 5);
            switch (ch) {
                case 1: addAssessmentToSection(); break;
                case 2: enterMarksForStudent();   break;
                case 3: viewAssessmentTemplate(); break;
                case 4: recalculateGrades();      break;
                case 5: viewStudentMarks();        break;
                case 0: return;
            }
            UI::pause();
        }
    }

    void addAssessmentToSection() {
        UI::banner("ADD ASSESSMENT TEMPLATE");
        string secID = UI::prompt("Section ID");
        if (!sectionExists(secID)) { UI::error("Section not found."); return; }

        Section* sec = findSection(secID);
        Course*  c   = sec ? findCourse(sec->getCourseID()) : nullptr;
        bool isLab   = c && c->getCourseType() == "Lab";

        cout << "  Assessment types: 1=Exam  2=Quiz  3=Assignment\n";
        if (isLab) UI::info("Lab courses: Exam marks are excluded from final grade.");
        int    type = UI::promptInt("Assessment type", 1, 3);
        double maxS = UI::promptDouble("Max score", 1, 1000);
        double wt   = UI::promptDouble("Weightage (%) in final grade", 0, 100);

        Assessment* a = nullptr;
        if      (type == 1) a = new Exam(0, maxS, wt);
        else if (type == 2) a = new Quiz(0, maxS, wt);
        else                a = new Assignment(0, maxS, wt);

        sectionAssessments[secID].push_back(a);

        // Propagate new template entry to already-enrolled students
        if (c) {
            for (const auto& enrolledSID : c->getEnrolledStudentIDs()) {
                Student* s = findStudent(enrolledSID);
                if (s) s->initStudentAssessmentsForSection(secID, sectionAssessments[secID]);
            }
        }
        UI::success("Assessment template entry added to section " + secID + ".");
    }

    void enterMarksForStudent() {
        UI::banner("ENTER MARKS FOR STUDENT");
        string secID = UI::prompt("Section ID");
        Section* sec = findSection(secID);
        if (!sec) { UI::error("Section not found."); return; }

        auto& templ = sectionAssessments[secID];
        if (templ.empty()) {
            UI::error("No assessment template for section " + secID + ". Add one first (Option 1)."); return;
        }

        string sid = UI::prompt("Student ID");
        Student* s = findStudent(sid);
        if (!s) { UI::error("Student not found."); return; }

        Course* c = findCourse(sec->getCourseID());
        if (!s->isEnrolledIn(sec->getCourseID())) {
            UI::error("Student is not enrolled in the course for this section."); return;
        }

        // Init per-student assessments if not yet done
        s->initStudentAssessmentsForSection(secID, templ);
        auto& asses = s->getStudentAssessmentsForSection(secID);

        cout << "\n  Assessments for " << s->getName() << " in section " << secID << ":\n";
        for (size_t i = 0; i < asses.size(); i++) {
            cout << "  " << (i + 1) << ". "; asses[i]->display();
        }

        int idx = UI::promptInt("Which assessment number to update", 1, (int)asses.size()) - 1;
        double raw = UI::promptDouble("Raw score", 0, asses[idx]->getMaxScore());
        asses[idx]->setRawScore(raw);

        // Auto-calculate and update final grade
        if (c) {
            double finalGrade = c->calculateFinalGrade(asses);
            s->setFinalGrade(c->getCourseID(), finalGrade);
            s->calculateGPA();

            // Scholarship probation check
            ScholarshipStudent* ss = dynamic_cast<ScholarshipStudent*>(s);
            if (ss) {
                if (ss->getGPA() < ss->getMinGPA()) {
                    ss->setStatusFlag("Probation");
                    UI::info("WARNING: " + s->getName() + " is on PROBATION. GPA: "
                             + to_string(ss->getGPA()).substr(0, 4));
                } else {
                    ss->setStatusFlag("Active");
                }
            }
            cout << "  Auto-calculated Final Grade: "
                 << fixed << setprecision(1) << finalGrade << "%\n";
        }
        UI::success("Marks updated for student " + sid + ".");
    }

    void viewAssessmentTemplate() {
        string secID = UI::prompt("Section ID");
        if (!sectionExists(secID)) { UI::error("Section not found."); return; }
        cout << "\n  Assessment TEMPLATE for section " << secID << ":\n";
        auto it = sectionAssessments.find(secID);
        if (it == sectionAssessments.end() || it->second.empty()) {
            UI::info("No assessment template defined for this section."); return;
        }
        for (auto a : it->second) a->display();
    }

    void viewStudentMarks() {
        string secID = UI::prompt("Section ID");
        Section* sec = findSection(secID);
        if (!sec) { UI::error("Section not found."); return; }

        string sid = UI::prompt("Student ID");
        Student* s = findStudent(sid);
        if (!s) { UI::error("Student not found."); return; }

        if (!s->hasStudentAssessmentsForSection(secID)) {
            UI::info("No marks recorded for this student in section " + secID + "."); return;
        }
        cout << "\n  Marks for " << s->getName() << " in section " << secID << ":\n";
        for (auto a : s->getStudentAssessmentsForSection(secID)) a->display();
    }

    void recalculateGrades() {
        UI::banner("RECALCULATE GRADES FOR SECTION");
        string secID = UI::prompt("Section ID");
        Section* sec = findSection(secID);
        if (!sec) { UI::error("Section not found."); return; }

        Course* c = findCourse(sec->getCourseID());
        if (!c) { UI::error("Course not found."); return; }

        auto it = sectionAssessments.find(secID);
        if (it == sectionAssessments.end() || it->second.empty()) {
            UI::error("No assessment template for section " + secID + "."); return;
        }
        auto& templ = it->second;

        bool any = false;
        for (auto s : students) {
            if (!s->isEnrolledIn(c->getCourseID())) continue;
            s->initStudentAssessmentsForSection(secID, templ);
            auto& asses   = s->getStudentAssessmentsForSection(secID);
            double grade  = c->calculateFinalGrade(asses);
            s->setFinalGrade(c->getCourseID(), grade);
            s->calculateGPA();

            ScholarshipStudent* ss = dynamic_cast<ScholarshipStudent*>(s);
            if (ss) {
                ss->setStatusFlag(ss->getGPA() < ss->getMinGPA() ? "Probation" : "Active");
                if (ss->getStudentStatus() == "Probation")
                    UI::info("Student " + s->getID() + " (" + s->getName() + ") → PROBATION.");
            }
            cout << "  " << s->getID() << " | " << s->getName()
                 << " | Final: " << fixed << setprecision(1) << grade << "%\n";
            any = true;
        }
        if (!any) UI::info("No enrolled students found for this section.");
        else UI::success("Grades recalculated.");
    }

    void menuScheduler() {
        while (true) {
            UI::banner("EXAM SCHEDULER");
            cout << "  1. Run Exam Scheduler\n"
                 << "  2. Run Conflict Solver\n"
                 << "  3. View Exam Schedule\n"
                 << "  0. Back\n";
            UI::divider();
            int ch = UI::promptInt("Choice", 0, 3);
            switch (ch) {
                case 1:
                    scheduler.scheduleExams(sections, courses, venues);
                    db.saveExamSchedule(sections);
                    UI::success("Exam schedule saved to exam_schedule.txt.");
                    break;
                case 2:
                    scheduler.resolveConflicts(sections, venues);
                    db.saveExamSchedule(sections);
                    UI::success("Conflict resolution complete. Schedule saved.");
                    break;
                case 3:
                    viewExamSchedule();
                    break;
                case 0: return;
            }
            UI::pause();
        }
    }

    void viewExamSchedule() {
        UI::banner("EXAM SCHEDULE");
        cout << left << setw(10) << "SecID" << setw(10) << "CourseID"
             << setw(24) << "Exam Slot" << "Venues\n";
        UI::divider();
        bool any = false;
        for (auto sec : sections) {
            const string& slot = sec->getExamTimeSlot();
            if (slot.empty() || slot == "UNSCHEDULED") continue;
            cout << left << setw(10) << sec->getSectionID()
                 << setw(10) << sec->getCourseID()
                 << setw(24) << slot;
            for (const auto& v : sec->getExamVenueIDs()) cout << v << " ";
            cout << "\n";
            any = true;
        }
        if (!any) UI::info("No exam schedule found. Run the scheduler first.");
    }

    void run() {
        loadAll();

        while (true) {
            UI::cls();
            UI::banner("FAST ACADEMIC OFFICE SYSTEM");
            cout << "  1. Student Management\n"
                 << "  2. Teacher Management\n"
                 << "  3. Course Management\n"
                 << "  4. Section Management\n"
                 << "  5. Venue Management\n"
                 << "  6. Smart Registration\n"
                 << "  7. Teacher Feedback\n"
                 << "  8. Grading  (Big Red Button)\n"
                 << "  9. Exam Scheduler\n"
                 << "  S. Save All Data\n"
                 << "  0. Exit\n";
            UI::divider();
            cout << "  Choice: ";
            cout.flush();
            string ch;
            if (!getline(cin, ch)) {
                cin.clear(); ch = "0"; // EOF → graceful exit
            }
            // Trim
            size_t a = ch.find_first_not_of(" \t\r\n");
            size_t b = ch.find_last_not_of(" \t\r\n");
            ch = (a == string::npos) ? "" : ch.substr(a, b - a + 1);

            if      (ch == "1") menuStudents();
            else if (ch == "2") menuTeachers();
            else if (ch == "3") menuCourses();
            else if (ch == "4") menuSections();
            else if (ch == "5") menuVenues();
            else if (ch == "6") menuRegistration();
            else if (ch == "7") menuFeedback();
            else if (ch == "8") menuGrading();
            else if (ch == "9") menuScheduler();
            else if (ch == "S" || ch == "s") { saveAll(); UI::pause(); }
            else if (ch == "0") {
                if (UI::promptYN("Save before exit?")) saveAll();
                cout << "\n  Goodbye. Academic chaos averted.\n\n";
                break;
            } else {
                UI::error("Invalid option. Choose 0-9 or S.");
                UI::pause();
            }
        }
    }
};
