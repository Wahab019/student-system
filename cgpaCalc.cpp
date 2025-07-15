#include <iostream>
#include <vector>
#include <string>
#include <iomanip>

using namespace std;

int main() {
    int numCourses;
    cout << "Enter number of courses: ";
    cin >> numCourses;

    vector<string> courseCodes(numCourses);
    vector<int> courseUnits(numCourses);
    vector<string> courseTypes(numCourses); // C, E, R
    vector<int> gradePoints(numCourses);
    vector<int> courseScores(numCourses);

    int totalUnits = 0;
    int totalCreditPoints = 0;

    cout << "\nEnter details for each course:\n";

    for (int i = 0; i < numCourses; ++i) {
        cout << "\nCourse " << i + 1 << ":\n";

        cout << "Course Code: ";
        cin >> ws; // to clear whitespace
        getline(cin, courseCodes[i]);

        cout << "Course Unit: ";
        cin >> courseUnits[i];

        cout << "Course Type (C = Compulsory, E = Elective, R = Required): ";
        cin >> courseTypes[i];

        cout << "Grade Point (0 - 5): ";
        cin >> gradePoints[i];

        courseScores[i] = courseUnits[i] * gradePoints[i];
        totalUnits += courseUnits[i];
        totalCreditPoints += courseScores[i];
    }

    double cgpa = static_cast<double>(totalCreditPoints) / totalUnits;

    // Print Result Table
    cout << "\n\n=========== CGPA RESULT ===========\n";
    cout << left << setw(10) << "Code"
         << setw(6) << "Unit"
         << setw(8) << "Type"
         << setw(10) << "G.P."
         << setw(10) << "Score" << endl;

    for (int i = 0; i < numCourses; ++i) {
        cout << left << setw(10) << courseCodes[i]
             << setw(6) << courseUnits[i]
             << setw(8) << courseTypes[i]
             << setw(10) << gradePoints[i]
             << setw(10) << courseScores[i] << endl;
    }

    cout << "\nTNU (Total Units): " << totalUnits;
    cout << "\nTCP (Total Credit Points): " << totalCreditPoints;
    cout << "\nCGPA: " << fixed << setprecision(2) << cgpa;

    // Optional Standing
    cout << "\nStanding: ";
    if (cgpa >= 4.5) cout << "First Class";
    else if (cgpa >= 3.5) cout << "Second Class Upper";
    else if (cgpa >= 2.5) cout << "Second Class Lower";
    else if (cgpa >= 1.5) cout << "Third Class";
    else cout << "Probation";

    cout << endl;

    return 0;
}