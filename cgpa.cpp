#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <fstream>
#include <thread>
#include <memory>
#include <unordered_map>
#include <sstream>
#include <chrono>
#include <mutex>
#include <algorithm>
#include <limits>
#include <regex>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    typedef int socklen_t;
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <fcntl.h>
    #define SOCKET int
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
    #define closesocket close
#endif

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

    void addCourse(const string& code, const string& type, int unit, int grade) {
        courses.emplace_back(code, type, unit, grade);
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

    string getStanding(double gpa) const {
        if (gpa >= 4.5) return "First Class";
        else if (gpa >= 3.5) return "Second Class Upper";
        else if (gpa >= 2.5) return "Second Class Lower";
        else if (gpa >= 1.5) return "Third Class";
        else return "Probation";
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
    string getLevel() const { return level; }
    string getSemester() const { return semester; }

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

// ===== HTTP Server Classes =====
class HttpRequest {
public:
    string method;
    string path;
    string version;
    unordered_map<string, string> headers;
    string body;
    
    static HttpRequest parse(const string& raw_request) {
        HttpRequest request;
        istringstream stream(raw_request);
        string line;
        
        // Parse request line
        if (getline(stream, line)) {
            istringstream line_stream(line);
            line_stream >> request.method >> request.path >> request.version;
        }
        
        // Parse headers
        while (getline(stream, line) && !line.empty() && line != "\r") {
            size_t colon_pos = line.find(':');
            if (colon_pos != string::npos) {
                string key = line.substr(0, colon_pos);
                string value = line.substr(colon_pos + 1);
                
                // Trim whitespace
                value.erase(0, value.find_first_not_of(" \t"));
                value.erase(value.find_last_not_of(" \t\r\n") + 1);
                
                request.headers[key] = value;
            }
        }
        
        // Parse body (if any)
        string body_line;
        while (getline(stream, body_line)) {
            request.body += body_line + "\n";
        }
        
        return request;
    }
};

class HttpResponse {
public:
    int status_code;
    string status_text;
    unordered_map<string, string> headers;
    string body;
    
    HttpResponse(int code = 200, const string& text = "OK") 
        : status_code(code), status_text(text) {
        headers["Content-Type"] = "text/html";
        headers["Server"] = "CGPA-Calculator-Server/1.0";
        headers["Connection"] = "close";
        headers["Access-Control-Allow-Origin"] = "*";
        headers["Access-Control-Allow-Methods"] = "GET, POST, OPTIONS";
        headers["Access-Control-Allow-Headers"] = "Content-Type";
    }
    
    string toString() const {
        string response = "HTTP/1.1 " + to_string(status_code) + " " + status_text + "\r\n";
        
        auto headers_copy = headers;
        headers_copy["Content-Length"] = to_string(body.length());
        
        for (const auto& header : headers_copy) {
            response += header.first + ": " + header.second + "\r\n";
        }
        
        response += "\r\n" + body;
        return response;
    }
};

class CGPAHttpServer {
private:
    SOCKET server_socket;
    int port;
    bool running;

public:
    CGPAHttpServer(int server_port) : port(server_port), running(false) {
        
#ifdef _WIN32
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            throw runtime_error("WSAStartup failed");
        }
#endif
        
        server_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (server_socket == INVALID_SOCKET) {
            throw runtime_error("Failed to create socket");
        }
        
        // Set socket options
        int opt = 1;
        setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, 
                  reinterpret_cast<const char*>(&opt), sizeof(opt));
    }
    
    ~CGPAHttpServer() {
        stop();
        closesocket(server_socket);
#ifdef _WIN32
        WSACleanup();
#endif
    }
    
    void start() {
        sockaddr_in server_addr{};
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port = htons(port);
        
        if (bind(server_socket, reinterpret_cast<sockaddr*>(&server_addr), 
                sizeof(server_addr)) == SOCKET_ERROR) {
            throw runtime_error("Failed to bind socket");
        }
        
        if (listen(server_socket, 10) == SOCKET_ERROR) {
            throw runtime_error("Failed to listen on socket");
        }
        
        running = true;
        cout << "CGPA Calculator HTTP Server started on port " << port << endl;
        cout << "Access the calculator at: http://localhost:" << port << endl;
        
        while (running) {
            sockaddr_in client_addr{};
            socklen_t client_len = sizeof(client_addr);
            
            SOCKET client_socket = accept(server_socket, 
                                        reinterpret_cast<sockaddr*>(&client_addr), 
                                        &client_len);
            
            if (client_socket != INVALID_SOCKET) {
                thread(&CGPAHttpServer::handleClient, this, client_socket).detach();
            }
        }
    }
    
    void stop() {
        running = false;
    }

private:
    void handleClient(SOCKET client_socket) {
        char buffer[4096];
        int bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
        
        if (bytes_received > 0) {
            buffer[bytes_received] = '\0';
            string raw_request(buffer);
            
            HttpRequest request = HttpRequest::parse(raw_request);
            HttpResponse response = handleRequest(request);
            
            string response_str = response.toString();
            send(client_socket, response_str.c_str(), response_str.length(), 0);
        }
        
        closesocket(client_socket);
    }
    
    HttpResponse handleRequest(const HttpRequest& request) {
        HttpResponse response;
        
        if (request.method == "OPTIONS") {
            response.status_code = 200;
            response.body = "";
            return response;
        }
        
        if (request.method == "GET") {
            if (request.path == "/" || request.path == "/index.html") {
                response.body = generateHomePage();
            } else {
                response = HttpResponse(404, "Not Found");
                response.body = "<h1>404 - Page Not Found</h1>";
            }
        } else if (request.method == "POST") {
            if (request.path == "/api/calculate") {
                response.headers["Content-Type"] = "application/json";
                response.body = handleCalculation(request.body);
            } else {
                response = HttpResponse(404, "Not Found");
                response.body = "<h1>404 - API Endpoint Not Found</h1>";
            }
        } else {
            response = HttpResponse(405, "Method Not Allowed");
            response.body = "<h1>405 - Method Not Allowed</h1>";
        }
        
        return response;
    }
    
    string generateHomePage() {
        return "<!DOCTYPE html>\n"
               "<html lang=\"en\">\n"
               "<head>\n"
               "    <meta charset=\"UTF-8\">\n"
               "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
               "    <title>CGPA Calculator</title>\n"
               "    <style>\n"
               "        * {\n"
               "            margin: 0;\n"
               "            padding: 0;\n"
               "            box-sizing: border-box;\n"
               "        }\n"
               "        \n"
               "        body {\n"
               "            font-family: 'Arial', sans-serif;\n"
               "            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);\n"
               "            min-height: 100vh;\n"
               "            padding: 20px;\n"
               "        }\n"
               "        \n"
               "        .container {\n"
               "            max-width: 1000px;\n"
               "            margin: 0 auto;\n"
               "            background: white;\n"
               "            border-radius: 10px;\n"
               "            box-shadow: 0 10px 30px rgba(0,0,0,0.3);\n"
               "            overflow: hidden;\n"
               "        }\n"
               "        \n"
               "        .header {\n"
               "            background: linear-gradient(135deg, #2c3e50 0%, #3498db 100%);\n"
               "            color: white;\n"
               "            padding: 30px;\n"
               "            text-align: center;\n"
               "        }\n"
               "        \n"
               "        .header h1 {\n"
               "            font-size: 2.5em;\n"
               "            margin-bottom: 10px;\n"
               "        }\n"
               "        \n"
               "        .header p {\n"
               "            font-size: 1.2em;\n"
               "            opacity: 0.9;\n"
               "        }\n"
               "        \n"
               "        .form-container {\n"
               "            padding: 30px;\n"
               "        }\n"
               "        \n"
               "        .form-group {\n"
               "            margin-bottom: 20px;\n"
               "        }\n"
               "        \n"
               "        label {\n"
               "            display: block;\n"
               "            margin-bottom: 8px;\n"
               "            font-weight: bold;\n"
               "            color: #333;\n"
               "        }\n"
               "        \n"
               "        select, input[type=\"number\"], input[type=\"text\"] {\n"
               "            width: 100%;\n"
               "            padding: 12px;\n"
               "            border: 2px solid #ddd;\n"
               "            border-radius: 6px;\n"
               "            font-size: 16px;\n"
               "            transition: border-color 0.3s;\n"
               "        }\n"
               "        \n"
               "        select:focus, input:focus {\n"
               "            outline: none;\n"
               "            border-color: #3498db;\n"
               "        }\n"
               "        \n"
               "        .semester-section {\n"
               "            background: #f8f9fa;\n"
               "            padding: 20px;\n"
               "            border-radius: 8px;\n"
               "            margin-bottom: 20px;\n"
               "            border: 2px solid #e9ecef;\n"
               "        }\n"
               "        \n"
               "        .semester-section h3 {\n"
               "            color: #2c3e50;\n"
               "            margin-bottom: 15px;\n"
               "            font-size: 1.3em;\n"
               "        }\n"
               "        \n"
               "        .course-grid {\n"
               "            display: grid;\n"
               "            grid-template-columns: 2fr 1fr 1fr 1fr auto;\n"
               "            gap: 10px;\n"
               "            align-items: center;\n"
               "            margin-bottom: 10px;\n"
               "            padding: 10px;\n"
               "            background: white;\n"
               "            border-radius: 6px;\n"
               "            border: 1px solid #ddd;\n"
               "        }\n"
               "        \n"
               "        .course-header {\n"
               "            font-weight: bold;\n"
               "            background: #3498db;\n"
               "            color: white;\n"
               "            padding: 12px 10px;\n"
               "            margin-bottom: 15px;\n"
               "        }\n"
               "        \n"
               "        .btn {\n"
               "            background: linear-gradient(135deg, #3498db 0%, #2980b9 100%);\n"
               "            color: white;\n"
               "            padding: 12px 24px;\n"
               "            border: none;\n"
               "            border-radius: 6px;\n"
               "            cursor: pointer;\n"
               "            font-size: 14px;\n"
               "            font-weight: bold;\n"
               "            transition: transform 0.2s;\n"
               "        }\n"
               "        \n"
               "        .btn:hover {\n"
               "            transform: translateY(-2px);\n"
               "        }\n"
               "        \n"
               "        .btn-add {\n"
               "            background: #27ae60;\n"
               "            margin-bottom: 20px;\n"
               "        }\n"
               "        \n"
               "        .btn-remove {\n"
               "            background: #e74c3c;\n"
               "            padding: 8px 12px;\n"
               "            font-size: 12px;\n"
               "        }\n"
               "        \n"
               "        .btn-calculate {\n"
               "            width: 100%;\n"
               "            font-size: 18px;\n"
               "            padding: 16px;\n"
               "            margin-top: 20px;\n"
               "        }\n"
               "        \n"
               "        .results {\n"
               "            margin-top: 30px;\n"
               "            padding: 20px;\n"
               "            background: #f8f9fa;\n"
               "            border-radius: 8px;\n"
               "            border-left: 4px solid #3498db;\n"
               "        }\n"
               "        \n"
               "        .result-item {\n"
               "            display: flex;\n"
               "            justify-content: space-between;\n"
               "            margin-bottom: 10px;\n"
               "            padding: 8px 0;\n"
               "            border-bottom: 1px solid #eee;\n"
               "        }\n"
               "        \n"
               "        .result-item:last-child {\n"
               "            border-bottom: none;\n"
               "            font-weight: bold;\n"
               "            font-size: 1.2em;\n"
               "        }\n"
               "        \n"
               "        .status {\n"
               "            padding: 15px;\n"
               "            border-radius: 6px;\n"
               "            margin-top: 15px;\n"
               "            text-align: center;\n"
               "            font-weight: bold;\n"
               "        }\n"
               "        \n"
               "        .status.first-class { background: #d4edda; color: #155724; }\n"
               "        .status.second-upper { background: #d1ecf1; color: #0c5460; }\n"
               "        .status.second-lower { background: #fff3cd; color: #856404; }\n"
               "        .status.third-class { background: #f8d7da; color: #721c24; }\n"
               "        .status.probation { background: #f8d7da; color: #721c24; }\n"
               "        \n"
               "        .hidden {\n"
               "            display: none;\n"
               "        }\n"
               "        \n"
               "        .semester-controls {\n"
               "            display: flex;\n"
               "            gap: 10px;\n"
               "            margin-bottom: 20px;\n"
               "        }\n"
               "        \n"
               "        .semester-controls select {\n"
               "            flex: 1;\n"
               "        }\n"
               "    </style>\n"
               "</head>\n"
               "<body>\n"
               "    <div class=\"container\">\n"
               "        <div class=\"header\">\n"
               "            <h1>CGPA Calculator</h1>\n"
               "            <p>Calculate your Cumulative Grade Point Average</p>\n"
               "        </div>\n"
               "        \n"
               "        <div class=\"form-container\">\n"
               "            <div class=\"semester-controls\">\n"
               "                <select id=\"levelSelect\">\n"
               "                    <option value=\"\">Select Level</option>\n"
               "                    <option value=\"100\">100 Level</option>\n"
               "                    <option value=\"200\">200 Level</option>\n"
               "                    <option value=\"300\">300 Level</option>\n"
               "                    <option value=\"400\">400 Level</option>\n"
               "                    <option value=\"500\">500 Level</option>\n"
               "                </select>\n"
               "                <select id=\"semesterSelect\">\n"
               "                    <option value=\"\">Select Semester</option>\n"
               "                    <option value=\"First\">First Semester</option>\n"
               "                    <option value=\"Second\">Second Semester</option>\n"
               "                </select>\n"
               "                <button type=\"button\" class=\"btn btn-add\" onclick=\"addSemester()\">Add Semester</button>\n"
               "            </div>\n"
               "            \n"
               "            <div id=\"semestersContainer\"></div>\n"
               "            \n"
               "            <button type=\"button\" class=\"btn btn-calculate\" onclick=\"calculateCGPA()\">Calculate CGPA</button>\n"
               "            \n"
               "            <div id=\"results\" class=\"results hidden\">\n"
               "                <h3>CGPA Results</h3>\n"
               "                <div id=\"resultsList\"></div>\n"
               "            </div>\n"
               "        </div>\n"
               "    </div>\n"
               "\n"
               "    <script>\n"
               "        var semesters = [];\n"
               "        var semesterCounter = 0;\n"
               "        \n"
               "        function addSemester() {\n"
               "            var level = document.getElementById('levelSelect').value;\n"
               "            var semester = document.getElementById('semesterSelect').value;\n"
               "            \n"
               "            if (!level || !semester) {\n"
               "                alert('Please select both level and semester');\n"
               "                return;\n"
               "            }\n"
               "            \n"
               "            var semesterData = {\n"
               "                id: semesterCounter++,\n"
               "                level: level,\n"
               "                semester: semester,\n"
               "                courses: []\n"
               "            };\n"
               "            \n"
               "            semesters.push(semesterData);\n"
               "            renderSemesters();\n"
               "            \n"
               "            // Reset selects\n"
               "            document.getElementById('levelSelect').value = '';\n"
               "            document.getElementById('semesterSelect').value = '';\n"
               "        }\n"
               "        \n"
               "        function renderSemesters() {\n"
               "            var container = document.getElementById('semestersContainer');\n"
               "            container.innerHTML = '';\n"
               "            \n"
               "            semesters.forEach(function(sem) {\n"
               "                var semesterDiv = document.createElement('div');\n"
               "                semesterDiv.className = 'semester-section';\n"
               "                semesterDiv.id = 'semester-' + sem.id;\n"
               "                \n"
               "                semesterDiv.innerHTML = `\n"
               "                    <h3>${sem.level} Level - ${sem.semester} Semester</h3>\n"
               "                    <div class=\"course-grid course-header\">\n"
               "                        <div>Course Code</div>\n"
               "                        <div>Units</div>\n"
               "                        <div>Type</div>\n"
               "                        <div>Grade</div>\n"
               "                        <div>Action</div>\n"
               "                    </div>\n"
               "                    <div id=\"courses-${sem.id}\"></div>\n"
               "                    <button type=\"button\" class=\"btn btn-add\" onclick=\"addCourse(${sem.id})\">Add Course</button>\n"
               "                `;\n"
               "                \n"
               "                container.appendChild(semesterDiv);\n"
               "                \n"
               "                // Add initial course\n"
               "                addCourse(sem.id);\n"
               "            });\n"
               "        }\n"
               "        \n"
               "        function addCourse(semesterId) {\n"
               "            var semester = semesters.find(s => s.id === semesterId);\n"
               "            var courseId = Date.now() + Math.random();\n"
               "            \n"
               "            var courseData = {\n"
               "                id: courseId,\n"
               "                code: '',\n"
               "                units: 0,\n"
               "                type: '',\n"
               "                grade: 0\n"
               "            };\n"
               "            \n"
               "            semester.courses.push(courseData);\n"
               "            renderCourses(semesterId);\n"
               "        }\n"
               "        \n"
               "        function renderCourses(semesterId) {\n"
               "            var semester = semesters.find(s => s.id === semesterId);\n"
               "            var container = document.getElementById('courses-' + semesterId);\n"
               "            container.innerHTML = '';\n"
               "            \n"
               "            semester.courses.forEach(function(course) {\n"
               "                var courseDiv = document.createElement('div');\n"
               "                courseDiv.className = 'course-grid';\n"
               "                \n"
               "                courseDiv.innerHTML = `\n"
               "                    <input type=\"text\" placeholder=\"Course Code\" value=\"${course.code}\" \n"
               "                           onchange=\"updateCourse(${semesterId}, ${course.id}, 'code', this.value)\">\n"
               "                    <input type=\"number\" min=\"1\" max=\"6\" placeholder=\"Units\" value=\"${course.units || ''}\" \n"
               "                           onchange=\"updateCourse(${semesterId}, ${course.id}, 'units', parseInt(this.value))\">\n"
               "                    <select onchange=\"updateCourse(${semesterId}, ${course.id}, 'type', this.value)\">\n"
               "                        <option value=\"\">Type</option>\n"
               "                        <option value=\"C\" ${course.type === 'C' ? 'selected' : ''}>C (Core)</option>\n"
               "                        <option value=\"E\" ${course.type === 'E' ? 'selected' : ''}>E (Elective)</option>\n"
               "                        <option value=\"R\" ${course.type === 'R' ? 'selected' : ''}>R (Required)</option>\n"
               "                    </select>\n"
               "                    <select onchange=\"updateCourse(${semesterId}, ${course.id}, 'grade', parseInt(this.value))\">\n"
               "                        <option value=\"\">Grade</option>\n"
               "                        <option value=\"5\" ${course.grade === 5 ? 'selected' : ''}>5 (A)</option>\n"
               "                        <option value=\"4\" ${course.grade === 4 ? 'selected' : ''}>4 (B)</option>\n"
               "                        <option value=\"3\" ${course.grade === 3 ? 'selected' : ''}>3 (C)</option>\n"
               "                        <option value=\"2\" ${course.grade === 2 ? 'selected' : ''}>2 (D)</option>\n"
               "                        <option value=\"1\" ${course.grade === 1 ? 'selected' : ''}>1 (E)</option>\n"
               "                        <option value=\"0\" ${course.grade === 0 ? 'selected' : ''}>0 (F)</option>\n"
               "                    </select>\n"
               "                    <button type=\"button\" class=\"btn btn-remove\" onclick=\"removeCourse(${semesterId}, ${course.id})\">Remove</button>\n"
               "                `;\n"
               "                container.appendChild(courseDiv);\n"
               "            });\n"
               "        }\n"
               "        \n"
               "        function updateCourse(semesterId, courseId, field, value) {\n"
               "            var semester = semesters.find(s => s.id === semesterId);\n"
               "            var course = semester.courses.find(c => c.id === courseId);\n"
               "            course[field] = value;\n"
               "        }\n"
               "        \n"
               "        function removeCourse(semesterId, courseId) {\n"
               "            var semester = semesters.find(s => s.id === semesterId);\n"
               "            semester.courses = semester.courses.filter(c => c.id !== courseId);\n"
               "            renderCourses(semesterId);\n"
               "        }\n"
               "        \n"
               "        function calculateCGPA() {\n"
               "            var data = {\n"
               "                semesters: semesters.map(sem => ({\n"
               "                    level: sem.level,\n"
               "                    semester: sem.semester,\n"
               "                    courses: sem.courses.filter(c => c.code && c.units && c.type && c.grade !== null)\n"
               "                }))\n"
               "            };\n"
               "            \n"
               "            if (data.semesters.length === 0) {\n"
               "                alert('Please add at least one semester with courses');\n"
               "                return;\n"
               "            }\n"
               "            \n"
               "            var xhr = new XMLHttpRequest();\n"
               "            xhr.open('POST', '/api/calculate', true);\n"
               "            xhr.setRequestHeader('Content-Type', 'application/json');\n"
               "            \n"
               "            xhr.onreadystatechange = function() {\n"
               "                if (xhr.readyState === 4 && xhr.status === 200) {\n"
               "                    var result = JSON.parse(xhr.responseText);\n"
               "                    displayResults(result);\n"
               "                }\n"
               "            };\n"
               "            \n"
               "            xhr.send(JSON.stringify(data));\n"
               "        }\n"
               "        \n"
               "        function displayResults(result) {\n"
               "            var resultsDiv = document.getElementById('results');\n"
               "            var resultsList = document.getElementById('resultsList');\n"
               "            \n"
               "            if (result.error) {\n"
               "                resultsList.innerHTML = '<div class=\"status probation\">Error: ' + result.error + '</div>';\n"
               "            } else {\n"
               "                var html = '';\n"
               "                \n"
               "                result.semesters.forEach(function(sem) {\n"
               "                    html += '<div class=\"result-item\">';\n"
               "                    html += '<span>' + sem.level + ' Level - ' + sem.semester + ' Semester GPA:</span>';\n"
               "                    html += '<span>' + sem.gpa.toFixed(2) + '/5.00</span>';\n"
               "                    html += '</div>';\n"
               "                });\n"
               "                \n"
               "                html += '<div class=\"result-item\">';\n"
               "                html += '<span>Overall CGPA:</span>';\n"
               "                html += '<span>' + result.cgpa.toFixed(2) + '/5.00</span>';\n"
               "                html += '</div>';\n"
               "                \n"
               "                html += '<div class=\"status ' + getStatusClass(result.cgpa) + '\">';\n"
               "                html += 'Classification: ' + result.classification;\n"
               "                html += '</div>';\n"
               "                \n"
               "                resultsList.innerHTML = html;\n"
               "            }\n"
               "            \n"
               "            resultsDiv.classList.remove('hidden');\n"
               "        }\n"
               "        \n"
               "        function getStatusClass(cgpa) {\n"
               "            if (cgpa >= 4.5) return 'first-class';\n"
               "            if (cgpa >= 3.5) return 'second-upper';\n"
               "            if (cgpa >= 2.5) return 'second-lower';\n"
               "            if (cgpa >= 1.5) return 'third-class';\n"
               "            return 'probation';\n"
               "        }\n"
               "    </script>\n"
               "</body>\n"
               "</html>";
    }
    
string handleCalculation(const string& json_body) {
    try {
        vector<unique_ptr<SemesterResult>> semesterResults;
        
        // More robust JSON parsing with better regex patterns
        regex semester_regex("\"level\"\\s*:\\s*\"(\\d+)\"\\s*,\\s*\"semester\"\\s*:\\s*\"([^\"]+)\"\\s*,\\s*\"courses\"\\s*:\\s*\\[([^\\]]*)\\]");
        regex course_regex("\\{\\s*\"id\"\\s*:\\s*[^,]+,\\s*\"code\"\\s*:\\s*\"([^\"]+)\"\\s*,\\s*\"units\"\\s*:\\s*(\\d+)\\s*,\\s*\"type\"\\s*:\\s*\"([^\"]+)\"\\s*,\\s*\"grade\"\\s*:\\s*(\\d+)\\s*\\}");
        
        sregex_iterator semester_iter(json_body.begin(), json_body.end(), semester_regex);
        sregex_iterator end;
        
        for (; semester_iter != end; ++semester_iter) {
            smatch semester_match = *semester_iter;
            string level = semester_match[1].str();
            string semester = semester_match[2].str();
            string courses_json = semester_match[3].str();
            
            // Skip empty courses
            if (courses_json.empty() || courses_json.find("\"code\"") == string::npos) {
                continue;
            }
            
            unique_ptr<SemesterResult> semResult;
            if (semester == "First") {
                semResult = make_unique<FirstSemesterResult>(level);
            } else {
                semResult = make_unique<SecondSemesterResult>(level);
            }
            
            // Parse courses with updated regex
            sregex_iterator course_iter(courses_json.begin(), courses_json.end(), course_regex);
            sregex_iterator course_end;
            
            bool hasValidCourses = false;
            for (; course_iter != course_end; ++course_iter) {
                smatch course_match = *course_iter;
                string code = course_match[1].str();
                int units = stoi(course_match[2].str());
                string type = course_match[3].str();
                int grade = stoi(course_match[4].str());
                
                // Only add courses with valid data
                if (!code.empty() && units > 0 && !type.empty()) {
                    semResult->addCourse(code, type, units, grade);
                    hasValidCourses = true;
                }
            }
            
            // Only add semester if it has valid courses
            if (hasValidCourses) {
                semesterResults.push_back(move(semResult));
            }
        }
        
        if (semesterResults.empty()) {
            return R"({"error": "No valid semester data found"})";
        }
        
        // Calculate CGPA
        double totalPoints = 0.0;
        int totalUnits = 0;
        ostringstream response;
        
        response << R"({"semesters": [)";
        
        for (size_t i = 0; i < semesterResults.size(); ++i) {
            if (i > 0) response << ",";
            
            auto& semester = semesterResults[i];
            double gpa = semester->calculateGPA();
            
            // Calculate semester units and points
            int semesterUnits = 0;
            int semesterPoints = 0;
            
            for (const auto& course : semester->getCourses()) {
                semesterUnits += course.getUnit();
                semesterPoints += course.getScore();
            }
            
            totalUnits += semesterUnits;
            totalPoints += semesterPoints;
            
            response << R"({"level": ")" << semester->getLevel() << R"(", )"
                    << R"("semester": ")" << semester->getSemester() << R"(", )"
                    << R"("gpa": )" << fixed << setprecision(2) << gpa << "}";
        }
        
        double cgpa = totalUnits > 0 ? totalPoints / totalUnits : 0.0;
        string classification = getClassification(cgpa);
        
        response << R"(], "cgpa": )" << fixed << setprecision(2) << cgpa
                << R"(, "classification": ")" << classification << R"("})";
        
        return response.str();
        
    } catch (const exception& e) {
        return R"({"error": ")" + string(e.what()) + R"("})";
    }
}
    
private:
    string getClassification(double cgpa) {
        if (cgpa >= 4.5) return "First Class";
        if (cgpa >= 3.5) return "Second Class Upper";
        if (cgpa >= 2.5) return "Second Class Lower";
        if (cgpa >= 1.5) return "Third Class";
        return "Probation";
    }
};

int main() {
    try {
        cout << "Starting CGPA Calculator HTTP Server..." << endl;
        
        CGPAHttpServer server(8080);
        
        cout << "Server will start on port 8080" << endl;
        cout << "Press Ctrl+C to stop the server" << endl;
        
        server.start();
        
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }
    
    return 0;
}