#include <iostream>
#include <vector>
#include <memory>
#include <fstream>

using namespace std;

// ABSTRACT BASE CLASS
class Student
{
protected:
    string name;
    int age;
    float score;

public:
    Student(string n, int a, float s) : name(n), age(a), score(s) {}

    virtual void displayInfo() const = 0;
    virtual string calculateGrade() const = 0;
    virtual string getType() const = 0;

    string getName() const { return name; }
    int getAge() const { return age; }
    float getScore() const { return score; }

    virtual ~Student() {}
};

// UNDERGRADUATE CLASS
class UndergraduateStudent : public Student
{
public:
    UndergraduateStudent(string n, int a, float s) : Student(n, a, s) {}

    void displayInfo() const override
    {
        cout << "Name: " << name << ", Age: " << age << ", Score: " << score
             << ", Grade: " << calculateGrade() << endl;
    }

    string calculateGrade() const override
    {
        if (score >= 70)
            return "A";
        else if (score >= 60)
            return "B";
        else if (score >= 50)
            return "C";
        else if (score >= 40)
            return "D";
        else
            return "F";
    }

    string getType() const override
    {
        return "Undergraduate";
    }
};

// GRADUATE CLASS
class GraduateStudent : public Student
{
public:
    GraduateStudent(string n, int a, float s) : Student(n, a, s) {}

    void displayInfo() const override
    {
        cout << "Name: " << name << ", Age: " << age << ", Score: " << score
             << ", Grade: " << calculateGrade() << endl;
    }

    string calculateGrade() const override
    {
        if (score >= 80)
            return "A";
        else if (score >= 70)
            return "B";
        else if (score >= 60)
            return "C";
        else
            return "F";
    }

    string getType() const override
    {
        return "Graduate";
    }
};

// POSTGRADUATE CLASS
class PostgraduateStudent : public Student
{
public:
    PostgraduateStudent(string n, int a, float s) : Student(n, a, s) {}

    void displayInfo() const override
    {
        cout << "Name: " << name << ", Age: " << age << ", Score: " << score
             << ", Grade: " << calculateGrade() << endl;
    }

    string calculateGrade() const override
    {
        if (score >= 80)
            return "A";
        else if (score >= 70)
            return "B";
        else if (score >= 60)
            return "C";
        else
            return "F";
    }

    string getType() const override
    {
        return "Postgraduate";
    }
};

// Save students to file
void saveToFile(const vector<shared_ptr<Student>> &students, const string &filename)
{
    ofstream file(filename);
    if (!file)
    {
        cout << "Error writing to file.\n";
        return;
    }

    for (const auto &s : students)
    {
        file << s->getType() << " " << s->getName() << " " << s->getAge() << " " << s->getScore() << endl;
    }

    file.close();
}

// Load students from file
void loadFromFile(vector<shared_ptr<Student>> &students, const string &filename)
{
    ifstream file(filename);
    if (!file)
    {
        cout << "No saved data found.\n";
        return;
    }

    string type, name;
    int age;
    float score;

    while (file >> type >> name >> age >> score)
    {
        if (type == "Undergraduate")
            students.push_back(make_shared<UndergraduateStudent>(name, age, score));
        else if (type == "Graduate")
            students.push_back(make_shared<GraduateStudent>(name, age, score));
        else if (type == "Postgraduate")
            students.push_back(make_shared<PostgraduateStudent>(name, age, score));
    }

    file.close();
}

// Add a new student
void addStudent(vector<shared_ptr<Student>> &students)
{
    string name;
    int age, type;
    float score;

    cout << "\nEnter Name: ";
    cin >> name;
    cout << "Enter Age: ";
    cin >> age;
    cout << "Enter Score (0-100): ";
    cin >> score;

    if (score < 0 || score > 100)
    {
        cout << "Invalid score entered.\n";
        return;
    }

    cout << "Select Student Type:\n1. Undergraduate\n2. Graduate\n3. Postgraduate\nChoice: ";
    cin >> type;

    if (type == 1)
    {
        students.push_back(make_shared<UndergraduateStudent>(name, age, score));
    }
    else if (type == 2)
    {
        students.push_back(make_shared<GraduateStudent>(name, age, score));
    }
    else if (type == 3)
    {
        students.push_back(make_shared<PostgraduateStudent>(name, age, score));
    }
    else
    {
        cout << "Invalid type selected.\n";
        return;
    }

    saveToFile(students, "students.txt"); // Save after adding
    cout << "Student added successfully.\n";
}

// Modify a student
void modifyStudent(vector<shared_ptr<Student>> &students)
{
    string targetName;
    cout << "\nEnter the name of the student to modify: ";
    cin >> targetName;

    for (auto &student : students)
    {
        if (student->getName() == targetName)
        {
            int newAge;
            float newScore;

            cout << "Enter new age: ";
            cin >> newAge;
            cout << "Enter new score: ";
            cin >> newScore;

            if (newScore < 0 || newScore > 100)
            {
                cout << "Invalid score. Must be between 0 and 100.\n";
                return;
            }

            if (student->getType() == "Undergraduate")
            {
                student = make_shared<UndergraduateStudent>(targetName, newAge, newScore);
            }
            else if (student->getType() == "Graduate")
            {
                student = make_shared<GraduateStudent>(targetName, newAge, newScore);
            }
            else if (student->getType() == "Postgraduate")
            {
                student = make_shared<PostgraduateStudent>(targetName, newAge, newScore);
            }

            cout << "Student modified successfully.\n";
            saveToFile(students, "students.txt");
            return;
        }
    }

    cout << "Student not found.\n";
}

// Delete a student
void deleteStudent(vector<shared_ptr<Student>> &students)
{
    string targetName;
    cout << "\nEnter the name of the student to delete: ";
    cin >> targetName;

    for (auto it = students.begin(); it != students.end(); ++it)
    {
        if ((*it)->getName() == targetName)
        {
            students.erase(it);
            cout << "Student deleted successfully.\n";
            saveToFile(students, "students.txt");
            return;
        }
    }

    cout << "Student not found.\n";
}

// View all students grouped
void viewStudents(const vector<shared_ptr<Student>> &students)
{
    if (students.empty())
    {
        cout << "\nNo students added yet.\n";
        return;
    }

    cout << "\n--- Undergraduate Students ---\n";
    for (const auto &student : students)
    {
        if (student->getType() == "Undergraduate")
        {
            student->displayInfo();
            cout << endl;
        }
    }

    cout << "\n--- Graduate Students ---\n";
    for (const auto &student : students)
    {
        if (student->getType() == "Graduate")
        {
            student->displayInfo();
            cout << endl;
        }
    }

    cout << "\n--- Postgraduate Students ---\n";
    for (const auto &student : students)
    {
        if (student->getType() == "Postgraduate")
        {
            student->displayInfo();
            cout << endl;
        }
    }
}

int main()
{
    vector<shared_ptr<Student>> students;

    loadFromFile(students, "students.txt");

    int choice;
    do
    {
        cout << "\n===== Student Management System =====\n";
        cout << "1. Add Student\n";
        cout << "2. View All Students\n";
        cout << "3. Modify Student\n";
        cout << "4. Delete Student\n";
        cout << "5. Exit\n";
        cout << "Enter your choice: ";
        cin >> choice;

        switch (choice)
        {
        case 1:
            addStudent(students);
            break;
        case 2:
            viewStudents(students);
            break;
        case 3:
            modifyStudent(students);
            break;
        case 4:
            deleteStudent(students);
            break;
        case 5:
            cout << "Exiting and saving data...\n";
            saveToFile(students, "students.txt");
            break;
        default:
            cout << "Invalid choice.\n";
        }
    } while (choice != 5);

    return 0;
}
