#pragma once
#include "Section.h"
#include "Course.h"
#include "Venue.h"
#include <vector>
#include <string>
#include <map>
#include <iostream>
#include <algorithm>
using namespace std;

// ============================================================
//  Scheduler — assigns venues + time slots to sections for exams
//  BUG FIX #5: venueOccupied is rebuilt from existing data before
//              resolveConflicts() so it works standalone (not just
//              after scheduleExams()).
// ============================================================
class Scheduler {
private:
    vector<string> timeSlots = {
        "Day1 08:00-10:00", "Day1 10:30-12:30", "Day1 13:30-15:30", "Day1 16:00-18:00",
        "Day2 08:00-10:00", "Day2 10:30-12:30", "Day2 13:30-15:30", "Day2 16:00-18:00",
        "Day3 08:00-10:00", "Day3 10:30-12:30", "Day3 13:30-15:30", "Day3 16:00-18:00",
        "Day4 08:00-10:00", "Day4 10:30-12:30", "Day4 13:30-15:30", "Day4 16:00-18:00",
        "Day5 08:00-10:00", "Day5 10:30-12:30", "Day5 13:30-15:30", "Day5 16:00-18:00"
    };

    map<string, vector<string>> venueOccupied; // venueID -> list of occupied slots

    bool isVenueFree(const string& vid, const string& slot) {
        auto& occ = venueOccupied[vid];
        return find(occ.begin(), occ.end(), slot) == occ.end();
    }

    void occupyVenue(const string& vid, const string& slot) {
        venueOccupied[vid].push_back(slot);
    }

    // BUG FIX #5: rebuild venueOccupied from existing scheduled sections
    void rebuildOccupied(const vector<Section*>& sections) {
        venueOccupied.clear();
        for (auto sec : sections) {
            if (sec->getExamTimeSlot().empty() || sec->getExamTimeSlot() == "UNSCHEDULED") continue;
            for (const auto& vid : sec->getExamVenueIDs())
                occupyVenue(vid, sec->getExamTimeSlot());
        }
    }

public:
    void scheduleExams(vector<Section*>& sections,
                       vector<Course*>&  courses,
                       vector<Venue*>&   venues) {
        cout << "\n  ╔═══════════════════════════════════════╗" << endl;
        cout << "  ║        EXAM SCHEDULING ENGINE         ║" << endl;
        cout << "  ╚═══════════════════════════════════════╝" << endl;

        // BUG FIX #5: start fresh but keep any already-scheduled slots
        rebuildOccupied(sections);

        for (auto sec : sections) {
            // Skip already scheduled sections
            if (!sec->getExamTimeSlot().empty() && sec->getExamTimeSlot() != "UNSCHEDULED") {
                cout << "  [SKIP] Section " << sec->getSectionID()
                     << " already scheduled at " << sec->getExamTimeSlot() << endl;
                continue;
            }

            Course* course = nullptr;
            for (auto c : courses)
                if (c->getCourseID() == sec->getCourseID()) { course = c; break; }

            if (!course) {
                cout << "  [!] Course " << sec->getCourseID() << " not found for section "
                     << sec->getSectionID() << ". Skipping." << endl;
                continue;
            }

            if (course->getExamDurationMinutes() == 0) {
                cout << "  [LAB] Section " << sec->getSectionID()
                     << " is a Lab course — no exam scheduled." << endl;
                continue;
            }

            int studentsLeft = sec->getStudentCount();
            if (studentsLeft <= 0) studentsLeft = course->getEnrollmentCount();
            if (studentsLeft <= 0) studentsLeft = 1; // fallback so we still schedule

            bool scheduled = false;
            for (size_t si = 0; si < timeSlots.size() && !scheduled; si++) {
                const string& slot = timeSlots[si];
                vector<string> assignedVenues;
                int remaining = studentsLeft;

                for (auto v : venues) {
                    if (remaining <= 0) break;
                    if (course->requiresComputers() && !v->getHasComputers()) continue;
                    if (!isVenueFree(v->getRoomID(), slot)) continue;
                    assignedVenues.push_back(v->getRoomID());
                    remaining -= v->getCapacity();
                }

                if (remaining <= 0) {
                    sec->setExamTimeSlot(slot);
                    sec->setExamVenues(assignedVenues);
                    for (const auto& vid : assignedVenues) occupyVenue(vid, slot);

                    cout << "  [OK] Section " << sec->getSectionID()
                         << " (" << course->getCourseID() << ")"
                         << " → Slot: " << slot << " | Venues: ";
                    for (const auto& v : assignedVenues) cout << v << " ";
                    cout << endl;
                    scheduled = true;
                }
            }

            if (!scheduled) {
                cout << "  [!!] CONFLICT: Cannot schedule section " << sec->getSectionID()
                     << " — not enough venue capacity. Flagging for review." << endl;
                sec->setExamTimeSlot("UNSCHEDULED");
            }
        }
        cout << "\n  Scheduling complete." << endl;
    }

    // BUG FIX #5: rebuild venueOccupied FIRST so conflict detection is accurate
    void resolveConflicts(vector<Section*>& sections, vector<Venue*>& venues) {
        cout << "\n  ── Conflict Solver Running ──" << endl;

        // Rebuild occupied map from existing schedule before detecting conflicts
        rebuildOccupied(sections);

        bool conflictFound = false;
        map<string, vector<string>> usage; // "slot|venueID" -> [sectionIDs]

        for (auto sec : sections) {
            if (sec->getExamTimeSlot().empty() || sec->getExamTimeSlot() == "UNSCHEDULED") continue;
            for (const auto& vid : sec->getExamVenueIDs()) {
                string key = sec->getExamTimeSlot() + "|" + vid;
                usage[key].push_back(sec->getSectionID());
            }
        }

        for (const auto& kv : usage) {
            if (kv.second.size() > 1) {
                conflictFound = true;
                cout << "  [CONFLICT] Slot+Venue [" << kv.first << "] has "
                     << kv.second.size() << " sections: ";
                for (const auto& s : kv.second) cout << s << " ";
                cout << endl;

                // Keep the first, reschedule the rest
                for (size_t i = 1; i < kv.second.size(); i++) {
                    Section* conflicted = nullptr;
                    for (auto sec : sections)
                        if (sec->getSectionID() == kv.second[i]) { conflicted = sec; break; }
                    if (!conflicted) continue;

                    // Free this section's venues from the occupied map before rescheduling
                    string oldSlot = conflicted->getExamTimeSlot();
                    for (const auto& vid : conflicted->getExamVenueIDs()) {
                        auto& occ = venueOccupied[vid];
                        occ.erase(remove(occ.begin(), occ.end(), oldSlot), occ.end());
                    }
                    conflicted->setExamVenues({});
                    conflicted->setExamTimeSlot("UNSCHEDULED");

                    string newSlot = "OVERFLOW";
                    vector<string> newVenues;
                    int need = conflicted->getStudentCount();
                    if (need <= 0) need = 1;

                    for (size_t si = 0; si < timeSlots.size(); si++) {
                        newVenues.clear();
                        int remaining = need;
                        for (auto v : venues) {
                            if (remaining <= 0) break;
                            if (!isVenueFree(v->getRoomID(), timeSlots[si])) continue;
                            newVenues.push_back(v->getRoomID());
                            remaining -= v->getCapacity();
                        }
                        if (remaining <= 0) { newSlot = timeSlots[si]; break; }
                    }

                    if (newSlot == "OVERFLOW") {
                        cout << "  [!!] Could not find a new slot for section "
                             << conflicted->getSectionID() << ". Manual intervention needed." << endl;
                        conflicted->setExamTimeSlot("UNSCHEDULED");
                    } else {
                        conflicted->setExamTimeSlot(newSlot);
                        conflicted->setExamVenues(newVenues);
                        for (const auto& vid : newVenues) occupyVenue(vid, newSlot);
                        cout << "  [RESOLVED] Section " << conflicted->getSectionID()
                             << " moved to slot: " << newSlot << endl;
                    }
                }
            }
        }

        if (!conflictFound) cout << "  No conflicts found." << endl;
    }

    bool assignVenueToSection(Section* sec, vector<Venue*>& venues,
                              int studentCount, bool needsComputers) {
        for (auto v : venues) {
            if (needsComputers && !v->getHasComputers()) continue;
            if (v->getCapacity() >= studentCount) {
                sec->setVenueID(v->getRoomID());
                return true;
            }
        }
        return false;
    }
};
