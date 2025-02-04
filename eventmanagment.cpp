#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <limits>
#include <functional> // For std::hash
#include <stdexcept>

using namespace std;

// -----------------------------
// Authentication Manager Claś̥ss
// -----------------------------
class AuthManager
{
private:
    string usersFile;

    // Basic password "hashing" using std::hash (not secure for production)
    string hashPassword(const string &password)
    {
        size_t hashVal = hash<string>{}(password);
        return to_string(hashVal);
    }

public:
    AuthManager(const string &filename) : usersFile(filename) {}

    bool signUp(const string &username, const string &password)
    {
        if (userExists(username))
        {
            return false;
        }
        string hashedPassword = hashPassword(password);
        ofstream outFile(usersFile, ios::app);
        if (!outFile)
        {
            throw runtime_error("Error: Cannot open users file for writing.");
        }
        outFile << username << " " << hashedPassword << "\n";
        return true;
    }

    bool logIn(const string &username, const string &password)
    {
        string hashedPassword = hashPassword(password);
        ifstream inFile(usersFile);
        if (!inFile)
        {
            throw runtime_error("Error: Cannot open users file for reading.");
        }
        string fileUsername, fileHashedPassword;
        while (inFile >> fileUsername >> fileHashedPassword)
        {
            if (fileUsername == username && fileHashedPassword == hashedPassword)
            {
                return true;
            }
        }
        return false;
    }

    bool userExists(const string &username)
    {
        ifstream inFile(usersFile);
        if (!inFile)
        {
            // File does not exist or cannot be opened—assume no users yet.
            return false;
        }
        string fileUsername, dummy;
        while (inFile >> fileUsername >> dummy)
        {
            if (fileUsername == username)
            {
                return true;
            }
        }
        return false;
    }
};

// -----------------------------
// Event Class
// -----------------------------
class Event
{
public:
    int id;
    string name;
    string date;
    string location;
    vector<string> attendees;

    Event() : id(0) {}
};

// -----------------------------
// Event Management Class
// -----------------------------
class EventManager
{
private:
    string eventsFile;

public:
    EventManager(const string &filename) : eventsFile(filename) {}

    vector<Event> loadEvents()
    {
        vector<Event> events;
        ifstream inFile(eventsFile);
        if (!inFile)
        {
            // If the file does not exist, return an empty list.
            return events;
        }

        string line;
        while (getline(inFile, line))
        {
            if (line.empty())
                continue;
            stringstream ss(line);
            Event e;
            string token;
            string attendeesList;

            // Expected format: id|name|date|location|attendee1,attendee2,...
            if (getline(ss, token, '|'))
            {
                e.id = stoi(token);
            }
            getline(ss, e.name, '|');
            getline(ss, e.date, '|');
            getline(ss, e.location, '|');
            getline(ss, attendeesList, '|');

            if (!attendeesList.empty())
            {
                stringstream attStream(attendeesList);
                string attendee;
                while (getline(attStream, attendee, ','))
                {
                    if (!attendee.empty())
                    {
                        e.attendees.push_back(attendee);
                    }
                }
            }
            events.push_back(e);
        }
        return events;
    }

    void saveEvents(const vector<Event> &events)
    {
        ofstream outFile(eventsFile);
        if (!outFile)
        {
            throw runtime_error("Error: Cannot open events file for writing.");
        }
        for (const auto &e : events)
        {
            outFile << e.id << "|"
                    << e.name << "|"
                    << e.date << "|"
                    << e.location << "|";
            for (size_t i = 0; i < e.attendees.size(); ++i)
            {
                outFile << e.attendees[i];
                if (i < e.attendees.size() - 1)
                    outFile << ",";
            }
            outFile << "\n";
        }
    }

    void createEvent()
    {
        vector<Event> events = loadEvents();
        Event newEvent;
        newEvent.id = (events.empty() ? 1 : events.back().id + 1);

        cout << "Enter event name: ";
        getline(cin, newEvent.name);
        cout << "Enter event date (YYYY-MM-DD): ";
        getline(cin, newEvent.date);
        cout << "Enter event location: ";
        getline(cin, newEvent.location);

        events.push_back(newEvent);
        saveEvents(events);
        cout << "Event created successfully with ID: " << newEvent.id << "\n";
    }

    void listEvents()
    {
        vector<Event> events = loadEvents();
        if (events.empty())
        {
            cout << "No events available.\n";
            return;
        }
        for (const auto &e : events)
        {
            cout << "\nID: " << e.id << "\n"
                << "Name: " << e.name << "\n"
                << "Date: " << e.date << "\n"
                << "Location: " << e.location << "\n"
                << "Attendees: " << e.attendees.size() << "\n";
            cout << "---------------------\n";
        }
    }

    void registerForEvent(const string &username)
    {
        vector<Event> events = loadEvents();
        if (events.empty())
        {
            cout << "No events to register for.\n";
            return;
        }
        int eventId;
        cout << "Enter the event ID to register: ";
        cin >> eventId;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        bool found = false;
        for (auto &e : events)
        {
            if (e.id == eventId)
            {
                bool alreadyRegistered = false;
                for (const auto &att : e.attendees)
                {
                    if (att == username)
                    {
                        alreadyRegistered = true;
                        break;
                    }
                }
                if (!alreadyRegistered)
                {
                    e.attendees.push_back(username);
                    cout << "Successfully registered for event '" << e.name << "'.\n";
                }
                else
                {
                    cout << "You are already registered for this event.\n";
                }
                found = true;
                break;
            }
        }
        if (!found)
        {
            cout << "Event with ID " << eventId << " not found.\n";
        }
        saveEvents(events);
    }
};

// -----------------------------
// Helper Functions for Menus
// -----------------------------
void showMainMenu()
{
    cout << "\n=== Advanced Event Management System ===\n";
    cout << "1. Sign Up\n";
    cout << "2. Log In\n";
    cout << "3. Exit\n";
    cout << "Enter your choice: ";
}

void showEventMenu()
{
    cout << "\n--- Event Menu ---\n";
    cout << "1. Create Event\n";
    cout << "2. List Events\n";
    cout << "3. Register for an Event\n";
    cout << "4. Log Out\n";
    cout << "Enter your choice: ";
}

// -----------------------------
// Main Function
// -----------------------------
int main()
{
    AuthManager auth("users.txt");
    EventManager eventMgr("events.txt");
    int choice;
    string currentUser;

    while (true)
    {
        showMainMenu();
        cin >> choice;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        if (choice == 1)
        {
            string username, password;
            cout << "Enter new username: ";
            cin >> username;
            cout << "Enter new password: ";
            cin >> password;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            try
            {
                if (auth.signUp(username, password))
                {
                    cout << "Sign up successful. You can now log in.\n";
                }
                else
                {
                    cout << "Username already exists. Try a different username.\n";
                }
            }
            catch (const runtime_error &e)
            {
                cout << e.what() << "\n";
            }
        }
        else if (choice == 2)
        {
            string username, password;
            cout << "Enter username: ";
            cin >> username;
            cout << "Enter password: ";
            cin >> password;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');

            try
            {
                if (auth.logIn(username, password))
                {
                    cout << "Login successful. Welcome, " << username << "!\n";
                    currentUser = username;
                    // Event management loop
                    while (true)
                    {
                        showEventMenu();
                        cin >> choice;
                        cin.ignore(numeric_limits<streamsize>::max(), '\n');
                        if (choice == 1)
                        {
                            eventMgr.createEvent();
                        }
                        else if (choice == 2)
                        {
                            eventMgr.listEvents();
                        }
                        else if (choice == 3)
                        {
                            eventMgr.registerForEvent(currentUser);
                        }
                        else if (choice == 4)
                        {
                            cout << "Logging out...\n";
                            break;
                        }
                        else
                        {
                            cout << "Invalid choice. Try again.\n";
                        }
                    }
                }
                else
                {
                    cout << "Login failed. Check your username and password.\n";
                }
            }
            catch (const runtime_error &e)
            {
                cout << e.what() << "\n";
            }
        }
        else if (choice == 3)
        {
            cout << "Exiting the system. Goodbye!\n";
            break;
        }
        else
        {
            cout << "Invalid choice. Please try again.\n";
        }
    }
    return 0;
}
