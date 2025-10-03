// library.cpp
#include <bits/stdc++.h>
#include <cassert>
using namespace std;

class Book {
public:
    int id;
    string title;
    string author;
    string isbn;
    int totalCopies;
    int availableCopies;

    Book() = default;
    Book(int id_, const string& t, const string& a, const string& i, int copies)
        : id(id_), title(t), author(a), isbn(i), totalCopies(copies), availableCopies(copies) {}

    void addCopies(int n) {
        if (n <= 0) return;
        totalCopies += n;
        availableCopies += n;
    }

    bool removeOneAvailable() {
        if (availableCopies <= 0) return false;
        --availableCopies;
        return true;
    }

    void returnOne() {
        if (availableCopies < totalCopies) ++availableCopies;
    }

    string brief() const {
        stringstream ss;
        ss << "[" << id << "] \"" << title << "\" by " << author
           << " (ISBN: " << isbn << ") - available: " << availableCopies
           << "/" << totalCopies;
        return ss.str();
    }
};

class User {
public:
    int id;
    string name;
    vector<int> borrowedBookIds;

    User() = default;
    User(int id_, const string& name_) : id(id_), name(name_) {}

    bool hasBorrowed(int bookId) const {
        return find(borrowedBookIds.begin(), borrowedBookIds.end(), bookId) != borrowedBookIds.end();
    }

    void borrowBook(int bookId) {
        if (!hasBorrowed(bookId)) borrowedBookIds.push_back(bookId);
    }

    bool returnBook(int bookId) {
        auto it = find(borrowedBookIds.begin(), borrowedBookIds.end(), bookId);
        if (it == borrowedBookIds.end()) return false;
        borrowedBookIds.erase(it);
        return true;
    }

    string brief() const {
        stringstream ss;
        ss << "[" << id << "] " << name << " - borrowed: ";
        if (borrowedBookIds.empty()) ss << "none";
        else {
            for (size_t i=0;i<borrowedBookIds.size();++i){
                if (i) ss << ", ";
                ss << borrowedBookIds[i];
            }
        }
        return ss.str();
    }
};

class Library {
private:
    vector<Book> books;
    vector<User> users;
    int nextBookId = 1;
    int nextUserId = 1;

    Book* findBookById(int id) {
        for (auto &b : books) if (b.id == id) return &b;
        return nullptr;
    }
    const Book* findBookByIdConst(int id) const {
        for (auto &b : books) if (b.id == id) return &b;
        return nullptr;
    }
    User* findUserById(int id) {
        for (auto &u : users) if (u.id == id) return &u;
        return nullptr;
    }

public:
    Library() = default;

    int addBook(const string& title, const string& author, const string& isbn, int copies) {
        Book b(nextBookId++, title, author, isbn, copies);
        books.push_back(b);
        return b.id;
    }

    bool removeBook(int bookId) {
        for (auto it = books.begin(); it != books.end(); ++it) {
            if (it->id == bookId) {
                // optional: prevent removing when borrowed (available < total)
                if (it->availableCopies < it->totalCopies) {
                    // Cannot remove since some copies are currently borrowed
                    return false;
                }
                books.erase(it);
                return true;
            }
        }
        return false;
    }

    int registerUser(const string& name) {
        User u(nextUserId++, name);
        users.push_back(u);
        return u.id;
    }

    bool borrowBook(int userId, int bookId) {
        User* user = findUserById(userId);
        Book* book = findBookById(bookId);
        if (!user || !book) return false;
        if (book->availableCopies <= 0) return false;
        if (user->hasBorrowed(bookId)) return false; // disallow duplicate borrow of same copy
        bool ok = book->removeOneAvailable();
        if (!ok) return false;
        user->borrowBook(bookId);
        return true;
    }

    bool returnBook(int userId, int bookId) {
        User* user = findUserById(userId);
        Book* book = findBookById(bookId);
        if (!user || !book) return false;
        bool removed = user->returnBook(bookId);
        if (!removed) return false;
        book->returnOne();
        return true;
    }

    vector<Book> searchByTitle(const string& q) const {
        vector<Book> out;
        string ql = q; transform(ql.begin(), ql.end(), ql.begin(), ::tolower);
        for (const auto &b : books) {
            string t = b.title; transform(t.begin(), t.end(), t.begin(), ::tolower);
            if (t.find(ql) != string::npos) out.push_back(b);
        }
        return out;
    }

    vector<Book> searchByAuthor(const string& q) const {
        vector<Book> out;
        string ql = q; transform(ql.begin(), ql.end(), ql.begin(), ::tolower);
        for (const auto &b : books) {
            string a = b.author; transform(a.begin(), a.end(), a.begin(), ::tolower);
            if (a.find(ql) != string::npos) out.push_back(b);
        }
        return out;
    }

    vector<Book> listAllBooks() const {
        return books;
    }

    vector<User> listAllUsers() const {
        return users;
    }

    const Book* getBook(int id) const { return findBookByIdConst(id); }
    const User* getUser(int id) const { return find_if(users.begin(), users.end(), [&](const User& u){ return u.id==id; }) == users.end() ? nullptr : &(*find_if(users.begin(), users.end(), [&](const User& u){ return u.id==id; })); }
    // note: getUser above used twice; we avoid using in core logic in this simple program
};

///// Small test suite using assert /////

void runTests() {
    Library lib;

    // Add books
    int b1 = lib.addBook("C++ Primer", "Stanley Lippman", "111-111", 2);
    int b2 = lib.addBook("The C++ Programming Language", "Bjarne Stroustrup", "222-222", 1);
    int b3 = lib.addBook("Clean Code", "Robert C. Martin", "333-333", 3);

    // Register users
    int u1 = lib.registerUser("Alice");
    int u2 = lib.registerUser("Bob");

    // Borrow success
    assert(lib.borrowBook(u1, b1) == true); // C++ Primer: avail 1/2
    assert(lib.borrowBook(u2, b1) == true); // C++ Primer: avail 0/2
    // now b1 should have 0 available
    const Book* book1 = lib.getBook(b1);
    assert(book1 && book1->availableCopies == 0);

    // Borrow should fail when none available
    assert(lib.borrowBook(u1, b1) == false);

    // Borrow b2 (only 1 copy)
    assert(lib.borrowBook(u1, b2) == true);
    const Book* book2 = lib.getBook(b2);
    assert(book2 && book2->availableCopies == 0);

    // Return b1 by u2
    assert(lib.returnBook(u2, b1) == true);
    book1 = lib.getBook(b1);
    assert(book1 && book1->availableCopies == 1);

    // Returning a book not borrowed should fail
    assert(lib.returnBook(u2, b2) == false);

    // Remove book that has copies borrowed should fail (b2 is borrowed by u1)
    assert(lib.removeBook(b2) == false);

    // Return b2 then remove should succeed
    assert(lib.returnBook(u1, b2) == true);
    assert(lib.removeBook(b2) == true);

    // Search tests
    auto found = lib.searchByTitle("clean");
    assert(found.size() == 1);
    assert(found[0].title == "Clean Code");

    cout << "All tests passed ?\n";
}

///// Simple interactive menu demo /////

void interactiveDemo() {
    Library lib;
    // seed some data
    int b1 = lib.addBook("C++ Primer", "Stanley Lippman", "111-111", 2);
    int b2 = lib.addBook("The C++ Programming Language", "Bjarne Stroustrup", "222-222", 1);
    int u1 = lib.registerUser("Alice");
    int u2 = lib.registerUser("Bob");

    cout << "Welcome.\n";
    cout << "Seeded books and users. Use the menu below or exit.\n\n";

    while (true) {
        cout << "\nMenu:\n"
             << "1) List books\n2) List users\n3) Add book\n4) Register user\n5) Borrow book\n6) Return book\n7) Search by title\n8) Run tests\n0) Exit\nChoice: ";
        int choice;
        if (!(cin >> choice)) { cin.clear(); string tmp; getline(cin, tmp); continue; }
        if (choice == 0) break;
        if (choice == 1) {
            auto books = lib.listAllBooks();
            cout << "Books:\n";
            for (auto &b : books) cout << "  " << b.brief() << "\n";
        } else if (choice == 2) {
            auto users = lib.listAllUsers();
            cout << "Users:\n";
            for (auto &u : users) cout << "  " << u.brief() << "\n";
        } else if (choice == 3) {
            string title, author, isbn;
            int copies;
            cout << "Title: "; cin.ignore(numeric_limits<streamsize>::max(), '\n'); getline(cin, title);
            cout << "Author: "; getline(cin, author);
            cout << "ISBN: "; getline(cin, isbn);
            cout << "Copies: "; cin >> copies;
            int id = lib.addBook(title, author, isbn, copies);
            cout << "Added book id = " << id << "\n";
        } else if (choice == 4) {
            cout << "User name: "; cin.ignore(numeric_limits<streamsize>::max(), '\n'); string name; getline(cin, name);
            int id = lib.registerUser(name);
            cout << "Registered user id = " << id << "\n";
        } else if (choice == 5) {
            cout << "User id: "; int uid; cin >> uid;
            cout << "Book id: "; int bid; cin >> bid;
            bool ok = lib.borrowBook(uid, bid);
            cout << (ok ? "Borrowed successfully\n" : "Failed to borrow (maybe no copies or invalid ids or already borrowed)\n");
        } else if (choice == 6) {
            cout << "User id: "; int uid; cin >> uid;
            cout << "Book id: "; int bid; cin >> bid;
            bool ok = lib.returnBook(uid, bid);
            cout << (ok ? "Returned successfully\n" : "Failed to return (maybe not borrowed or invalid ids)\n");
        } else if (choice == 7) {
            cout << "Search term (title): "; cin.ignore(numeric_limits<streamsize>::max(), '\n'); string q; getline(cin, q);
            auto res = lib.searchByTitle(q);
            cout << "Found " << res.size() << " results:\n";
            for (auto &b : res) cout << "  " << b.brief() << "\n";
        } else if (choice == 8) {
            runTests();
        } else {
            cout << "Invalid choice\n";
        }
    }
    cout << "Goodbye.\n";
}

int main(int argc, char** argv) {
    if (argc > 1) {
        string arg = argv[1];
        if (arg == "--test") {
            runTests();
            return 0;
        }
    }
    interactiveDemo();
    return 0;
}

