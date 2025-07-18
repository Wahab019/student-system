#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <fstream>

using namespace std;

// ===== Course Class (Encapsulation) =====
class Course {
private:
    string code, type;
    int unit, grade;
public:
    Course(string c, string t, int u, int g)
        : code(c), type(t), unit(u), grade(g) {}

    int getScore() const { return unit * grade; }
    int getUnit() const { return unit; }
    int getGrade() const { return grade; }
    string getCode() const { return code; }
    string getType() const { return type; }

    void display(ofstream& outFile) const {
        outFile << left << setw(10) << code
                << setw(6) << unit
                << setw(8) << type
                << setw(10) << grade
                << setw(10) << getScore() << endl;
    }
};

// ===== Base Class for SemesterResult (Abstraction) =====
class SemesterResult {
protected:
    string level, semester;
    vector<Course> courses;

public:
    SemesterResult(const string& lvl, const string& sem)
        : level(lvl), semester(sem) {}

    virtual void inputCourses() {
        int num;
        cout << "Enter number of courses: ";
        cin >> num;

        for (int i = 0; i < num; ++i) {
            string code, type;
            int unit, grade;

            cout << "\nCourse " << i + 1 << ":\n";
            cout << "Course Code: ";
            cin >> ws;
            getline(cin, code);
            cout << "Course Unit: ";
            cin >> unit;
            cout << "Course Type (C, E, R): ";
            cin >> type;
            cout << "Grade Point (0 - 5): ";
            cin >> grade;

            courses.emplace_back(code, type, unit, grade);
        }
    }

    virtual double calculateGPA() const {
        int totalUnits = 0, totalPoints = 0;

        for (const auto& course : courses) {
            totalUnits += course.getUnit();
            totalPoints += course.getScore();
        }

        return totalUnits == 0 ? 0.0 : static_cast<double>(totalPoints) / totalUnits;
    }

    virtual void printStanding(double gpa) const {
        if (gpa >= 4.5) cout << "Standing: First Class\n";
        else if (gpa >= 3.5) cout << "Standing: Second Class Upper\n";
        else if (gpa >= 2.5) cout << "Standing: Second Class Lower\n";
        else if (gpa >= 1.5) cout << "Standing: Third Class\n";
        else cout << "Standing: Probation\n";
    }

    virtual void saveToFile(ofstream& outFile) const {
        outFile << "\n\n===== " << level << " Level, " << semester << " Semester =====\n";
        outFile << left << setw(10) << "Code"
                << setw(6) << "Unit"
                << setw(8) << "Type"
                << setw(10) << "Grade"
                << setw(10) << "Score" << endl;

        for (const auto& course : courses) {
            course.display(outFile);
        }

        double gpa = calculateGPA();
        outFile << "\nGPA: " << fixed << setprecision(2) << gpa << " / 5.00\n";
    }

    vector<Course> getCourses() const { return courses; }

    virtual ~SemesterResult() {}
};

// ===== Inheritance + Polymorphism =====
class FirstSemesterResult : public SemesterResult {
public:
    FirstSemesterResult(const string& lvl) : SemesterResult(lvl, "First") {}

    void printStanding(double gpa) const override {
        cout << "First Semester Result\n";
        SemesterResult::printStanding(gpa);
    }
};

class SecondSemesterResult : public SemesterResult {
public:
    SecondSemesterResult(const string& lvl) : SemesterResult(lvl, "Second") {}

    void printStanding(double gpa) const override {
        cout << "Second Semester Result\n";
        SemesterResult::printStanding(gpa);
    }
};

// ===== Main Function =====
int main() {
    vector<SemesterResult*> semesters;
    char another;

    do {
        string level, sem;
        cout << "\nEnter level (e.g., 100, 200): ";
        cin >> level;
        cout << "Enter semester (first/second): ";
        cin >> sem;

        SemesterResult* result;
        if (sem == "first")
            result = new FirstSemesterResult(level);
        else
            result = new SecondSemesterResult(level);

        result->inputCourses();
        double gpa = result->calculateGPA();

        cout << "\nGPA: " << fixed << setprecision(2) << gpa << " / 5.00\n";
        result->printStanding(gpa);

        semesters.push_back(result);

        cout << "\nDo you want to enter another semester? (y/n): ";
        cin >> another;

    } while (another == 'y' || another == 'Y');

    // Save to file and calculate cumulative CGPA
    ofstream outFile("cgpa_results.txt", ios::app);
    int grandUnits = 0, grandPoints = 0;

    for (auto s : semesters) {
        s->saveToFile(outFile);
        for (auto& c : s->getCourses()) {
            grandUnits += c.getUnit();
            grandPoints += c.getScore();
        }
        delete s;
    }

    if (grandUnits > 0) {
        double cgpa = static_cast<double>(grandPoints) / grandUnits;
        outFile << "\n=========== CUMULATIVE CGPA ===========\n";
        outFile << "Total Units: " << grandUnits << "\n";
        outFile << "Total Credit Points: " << grandPoints << "\n";
        outFile << "Cumulative CGPA: " << fixed << setprecision(2) << cgpa << " / 5.00\n";
    }

    outFile.close();
    cout << "\nAll results saved to 'cgpa_results.txt'.\n";
    return 0;
}