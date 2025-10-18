// IvanIvanov.cpp
// -----------------
// In‐memory file system with Flyweight, Iterator, and Visitor patterns.
// Author: Name Surname
// Date:   2025-05-01

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <map>
#include <sstream>
#include <cmath>
#include <locale>

using namespace std;

// --------------------
// Flyweight: shared immutable file metadata
// --------------------
struct FileProperties {
    string extension;   // e.g. "cpp", "txt"
    bool readOnly;      // true if read-only
    string owner;       // owner name
    string group;       // group name

    FileProperties(const string& ext, bool ro,
                   const string& o, const string& g)
      : extension(ext), readOnly(ro), owner(o), group(g) {}
};

class FilePropertiesFactory {
    unordered_map<string, shared_ptr<FileProperties>> cache;
    // Build a unique key from all four fields
    string makeKey(const string& ext, bool ro,
                   const string& o, const string& g) const {
        return ext + "|" + (ro ? "T" : "F") + "|" + o + "|" + g;
    }

public:
    // Return a shared FileProperties instance
    shared_ptr<FileProperties>
    get(const string& ext, bool ro,
        const string& owner, const string& group)
    {
        string key = makeKey(ext, ro, owner, group);
        auto it = cache.find(key);
        if (it != cache.end())
            return it->second;
        auto fp = make_shared<FileProperties>(ext, ro, owner, group);
        cache[key] = fp;
        return fp;
    }
};

// --------------------
// Composite & Visitor
// --------------------
class File;
class Directory;

// Visitor interface
struct Visitor {
    virtual void visit(File& f) = 0;
    virtual void visit(Directory& d) = 0;
    virtual ~Visitor() = default;
};

// Abstract base for files and directories
class Node {
protected:
    string name;
public:
    Node(string n) : name(move(n)) {}
    virtual ~Node() = default;

    const string& getName() const { return name; }
    virtual void accept(Visitor& v) = 0;
};

// Leaf: a file
class File : public Node {
    double sizeKB;
    shared_ptr<FileProperties> props;
public:
    File(const string& name, double sz,
         shared_ptr<FileProperties> p)
      : Node(name), sizeKB(sz), props(move(p)) {}

    double getSize() const { return sizeKB; }
    void accept(Visitor& v) override {
        v.visit(*this);
    }
};

// Composite: a directory holding children
class Directory : public Node {
    vector<shared_ptr<Node>> children;
public:
    Directory(const string& name) : Node(name) {}

    void add(shared_ptr<Node> c) {
        children.push_back(move(c));
    }
    const vector<shared_ptr<Node>>& getChildren() const {
        return children;
    }

    void accept(Visitor& v) override {
        v.visit(*this);
        for (auto& c : children)
            c->accept(v);
    }

    // Iterator for Linux-style tree printing
    struct Entry {
        Node* node;
        int depth;
        vector<bool> hasMoreSiblings;
        bool isLast;
    };
    class Iterator {
        vector<Entry> entries;
        size_t idx = 0;
    public:
        Iterator(Directory* root) {
            build(root, 0, {});
        }
        bool hasNext() const { return idx < entries.size(); }
        Entry next() { return entries[idx++]; }

    private:
        void build(Directory* dir, int depth,
                   vector<bool> prefix)
        {
            const auto& ch = dir->getChildren();
            for (size_t i = 0, n = ch.size(); i < n; ++i) {
                bool last = (i + 1 == n);
                entries.push_back({ ch[i].get(), depth, prefix, last });
                if (auto dp = dynamic_cast<Directory*>(ch[i].get())) {
                    auto np = prefix;
                    np.push_back(!last);
                    build(dp, depth + 1, np);
                }
            }
        }
    };

    unique_ptr<Iterator> createIterator() {
        return make_unique<Iterator>(this);
    }
};

// Visitor that sums file sizes
class SizeVisitor : public Visitor {
    double total = 0;
public:
    void visit(File& f) override {
        total += f.getSize();
    }
    void visit(Directory&) override {
        // no-op; recursion via accept()
    }
    double getTotal() const { return total; }
};

// --------------------
// Utility to format sizes:
//   • round to 2 decimals
//   • strip trailing zeros & dot
// --------------------
string formatSize(double sz) {
    sz = round(sz * 100.0) / 100.0;
    ostringstream os;
    os << fixed << setprecision(2) << sz;
    string s = os.str();  // e.g. "282.00", "282.50", "282.68"
    while (!s.empty() && s.back() == '0') s.pop_back();
    if (!s.empty() && s.back() == '.') s.pop_back();
    return s;
}

// --------------------
// Main
// --------------------
int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    setlocale(LC_ALL, "");

    int N; cin >> N;
    string line;
    getline(cin, line); // consume newline

    auto root = make_shared<Directory>(".");
    map<int, shared_ptr<Directory>> dirs;
    dirs[0] = root;

    FilePropertiesFactory fpFactory;

    // Read commands
    for (int read = 0; read < N; ){
        if (!getline(cin, line)) break;
        if (line.empty()) continue;

        istringstream ss(line);
        string cmd; ss >> cmd;

        if (cmd == "DIR") {
            int id, parent; string name;
            vector<string> toks; string tok;
            ss.clear(); ss.str(line);
            while (ss >> tok) toks.push_back(tok);

            if (toks.size() == 3) {
                id     = stoi(toks[1]);
                parent = 0;
                name   = toks[2];
            } else {
                id     = stoi(toks[1]);
                parent = stoi(toks[2]);
                name   = toks[3];
            }
            auto d = make_shared<Directory>(name);
            dirs[id] = d;
            dirs[parent]->add(d);
        }
        else if (cmd == "FILE") {
            int parent; char ro;
            string owner, group, nameext;
            double sz;
            ss >> parent >> ro >> owner >> group >> sz >> nameext;

            auto pos = nameext.rfind('.');
            string ext = (pos != string::npos
                             ? nameext.substr(pos+1)
                             : "");
            auto props = fpFactory.get(ext, ro=='T', owner, group);
            auto f = make_shared<File>(nameext, sz, props);
            dirs[parent]->add(f);
        }
        ++read;
    }

    // Compute total size
    SizeVisitor sv;
    root->accept(sv);
    cout << "total: " << formatSize(sv.getTotal()) << "KB\n";

    // Print tree
    cout << root->getName() << "\n";
    auto it = root->createIterator();
    while (it->hasNext()) {
        auto e = it->next();
        for (int d = 0; d < e.depth; ++d)
            cout << (e.hasMoreSiblings[d] ? "│   " : "    ");
        cout << (e.isLast ? "└── " : "├── ")
             << e.node->getName();
        if (auto fp = dynamic_cast<File*>(e.node)) {
            cout << " (" << formatSize(fp->getSize()) << "KB)";
        }
        cout << "\n";
    }
    return 0;
}
