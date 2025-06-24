#include <bits/stdc++.h>
using namespace std;

class File {
    string name;
    map<string, File*> mapForSubFile;
    bool isFile;
    File* parent;

public:
    File(const string &name, bool isFile, File* parent = nullptr) {
        this->name = name;
        this->isFile = isFile;
        this->parent = parent;
    }

    ~File() {
        for (auto& pair : mapForSubFile) {
            delete pair.second;
        }
    }

    bool addFile(File* fileToAdd) {
        if (isFile || 
        mapForSubFile.find(fileToAdd->name) != mapForSubFile.end()) {
            return false;
        }

        fileToAdd->parent = this;
        mapForSubFile[fileToAdd->name] = fileToAdd;
        return true;
    }
    
    bool isFileInDir(string fileName) {
        return mapForSubFile.find(fileName) != mapForSubFile.end();
    }
    
    File* getFileInDir(string fileName) {
        auto it = mapForSubFile.find(fileName);
        return (it != mapForSubFile.end()) ? it->second : nullptr;
    }
    
    bool getIsFile() {
        return isFile;
    }
    
    string getName() {
        return name;
    }
    
    File* getParent() {
        return parent;
    }
    
    vector<File*> getChildsNode() {
        vector<File*> children;
        for (auto& pair : mapForSubFile) {
            children.push_back(pair.second);
        }
        return children;
    }
    
    vector<string> getChildNames() {
        vector<string> names;
        for (auto& pair : mapForSubFile) {
            names.push_back(pair.first);
        }
        return names;
    }
    
    string getFullPath() {
        if (parent == nullptr) {
            return "/";
        }
        
        string parentPath = parent->getFullPath(); // Fixed: should use -> not .
        if (parentPath == "/") {
            return "/" + name;
        }
        
        return parentPath + "/" + name;
    }
    
    // Alias methods for consistency
    File* getChild(string name) {
        return getFileInDir(name);
    }
    
    bool addChild(File* child) {
        return addFile(child);
    }
};

class FileSystem {
    File* root;
    File* currentDir; 
    
    vector<string> splitPath(const string &path) { // Fixed: spiltPath -> splitPath
        vector<string> parts;
        if (path.empty()) return parts; // Fixed: parths -> parts
        
        stringstream ss(path); 
        string part;
        while (getline(ss, part, '/')) {
            if (!part.empty()) {
                parts.push_back(part);
            }
        }
        
        return parts; // Fixed: parths -> parts
    }
    
    File* navigateToPath(const string &path, bool createIfNotExist = false) { // Fixed: const string *path -> const string &path
        vector<string> parts = splitPath(path); // Fixed: spiltPath -> splitPath
        
        File* curr;
        if (path.empty() || path[0] == '/') {
            curr = root;
        } else {
            curr = currentDir;
        }
        
        for (string part : parts) {
            if (part == "..") {
                if (curr->getParent() != nullptr) {
                    curr = curr->getParent();
                }
                continue;
            }
            
            if (part == ".") {
                continue;
            }
            
            File* next = curr->getChild(part);
            if (next == nullptr) {
                if (createIfNotExist && !curr->getIsFile()) {
                    next = new File(part, false, curr);
                    curr->addChild(next);
                } else {
                    return nullptr;
                }
            }
            
            curr = next;
        }
        
        return curr;
    }
    
    void findMatchingPaths(File* curr, vector<string> &parts, int partIndex, string currentPath, vector<string> &result) {
        
        if (partIndex >= parts.size()) {
            result.push_back(currentPath);
            return;
        }
        
        string part = parts[partIndex];
        if (part == "*") {
            vector<File*> children = curr->getChildsNode();
            for (File* child : children) {
                if (!child->getIsFile()) { // Only match directories
                    string newPath = currentPath.empty() ? child->getName() : currentPath + "/" + child->getName();
                    findMatchingPaths(child, parts, partIndex + 1, newPath, result);
                }
            }
        } else if (part == "." || part == "..") {
            // Handle special directory references
            File* target = curr;
            if (part == "..") {
                target = curr->getParent();
                if (target == nullptr) target = curr; // Stay at root if no parent
            }
            findMatchingPaths(target, parts, partIndex + 1, currentPath, result);
        } else {
            File* child = curr->getChild(part);
            if (child != nullptr && !child->getIsFile()) {
                string newPath = currentPath.empty() ? child->getName() : currentPath + "/" + child->getName();
                findMatchingPaths(child, parts, partIndex + 1, newPath, result);
            }
        }
    }
    
public:
    FileSystem() {
        root = new File("/", false);
        currentDir = root;
    }
    
    ~FileSystem() {
        delete root;
    }
    
    bool mkdir(string path) {
        if (path.empty()) return false;
        
        File* target = navigateToPath(path, true);
        return target != nullptr;
    }
    
    bool chdir(const string &path) {
        File* target = navigateToPath(path);
        if (target == nullptr || target->getIsFile()) { // Fixed: target.getIsFile() -> target->getIsFile()
            return false;
        }
        
        currentDir = target;
        return true;
    }
    
    string getCurrentPath() {
        return currentDir->getFullPath();
    }
    
    vector<string> getFiles(const string &path) {
        vector<string> results;
        if (path.empty()) return results;
        
        vector<string> parts = splitPath(path); // Fixed: spiltPath -> splitPath
        if (parts.empty()) return results;
        
        File* startDir;
        if (path[0] == '/') {
            startDir = root;
        } else {
            startDir = currentDir;
        }
        
        findMatchingPaths(startDir, parts, 0, "", results);
        
        if (path[0] == '/') {
            for (string& result : results) {
                result = "/" + result;
            }
        }
        
        return results;
    }
    
    // Additional utility methods
    vector<string> listCurrentDirectory() {
        return currentDir->getChildNames();
    }
    
    bool createFile(const string& path) {
        if (path.empty()) return false;
        
        size_t lastSlash = path.find_last_of('/');
        string dirPath, fileName;
        
        if (lastSlash == string::npos) {
            dirPath = "";
            fileName = path;
        } else {
            dirPath = path.substr(0, lastSlash);
            fileName = path.substr(lastSlash + 1);
        }
        
        File* parentDir = navigateToPath(dirPath, false);
        if (parentDir == nullptr || parentDir->getIsFile()) {
            return false;
        }
        
        File* newFile = new File(fileName, true, parentDir);
        return parentDir->addChild(newFile);
    }
    
    bool pathExists(const string& path) {
        return navigateToPath(path) != nullptr;
    }
};

int main() {
    FileSystem fs;
    
    cout << "=== File System Testing ===" << endl;
    cout << "Current directory: " << fs.getCurrentPath() << endl << endl;
    
    // Test 1: Basic mkdir functionality
    cout << "Test 1: Creating directories" << endl;
    cout << "mkdir('/home/user/documents'): " << (fs.mkdir("/home/user/documents") ? "SUCCESS" : "FAILED") << endl;
    cout << "mkdir('temp'): " << (fs.mkdir("temp") ? "SUCCESS" : "FAILED") << endl;
    cout << "mkdir('projects/cpp'): " << (fs.mkdir("projects/cpp") ? "SUCCESS" : "FAILED") << endl;
    cout << "mkdir('projects/python'): " << (fs.mkdir("projects/python") ? "SUCCESS" : "FAILED") << endl;
    
    // Test 2: List root directory
    cout << "\nTest 2: Root directory contents:" << endl;
    vector<string> rootContents = fs.listCurrentDirectory();
    for (const string& item : rootContents) {
        cout << "- " << item << endl;
    }
    
    // Test 3: Change directory (absolute path)
    cout << "\nTest 3: Changing directories" << endl;
    cout << "chdir('/home/user'): " << (fs.chdir("/home/user") ? "SUCCESS" : "FAILED") << endl;
    cout << "Current directory: " << fs.getCurrentPath() << endl;
    
    // Test 4: Change directory (relative path)
    cout << "chdir('documents'): " << (fs.chdir("documents") ? "SUCCESS" : "FAILED") << endl;
    cout << "Current directory: " << fs.getCurrentPath() << endl;
    
    // Test 5: Navigate with .. and .
    cout << "\nTest 5: Navigation with .. and ." << endl;
    cout << "chdir('..'): " << (fs.chdir("..") ? "SUCCESS" : "FAILED") << endl;
    cout << "Current directory: " << fs.getCurrentPath() << endl;
    cout << "chdir('.'): " << (fs.chdir(".") ? "SUCCESS" : "FAILED") << endl;
    cout << "Current directory: " << fs.getCurrentPath() << endl;
    
    // Test 6: Go back to root and create test structure for pattern matching
    cout << "\nTest 6: Setting up pattern matching test" << endl;
    fs.chdir("/");
    fs.mkdir("/ABC/test1/DC");
    fs.mkdir("/ABC/test2/DC");
    fs.mkdir("/ABC/direct/DC");
    fs.mkdir("/ABC/another/DC");
    cout << "Created test structure for pattern matching" << endl;
    
    // Test 7: Pattern matching with wildcards
    cout << "\nTest 7: Pattern matching" << endl;
    vector<string> matches = fs.getFiles("/ABC/*/DC");
    cout << "Pattern '/ABC/*/DC' matches (" << matches.size() << " results):" << endl;
    for (const string& match : matches) {
        cout << "- " << match << endl;
    }
    
    // Test 8: Create and test files
    cout << "\nTest 8: File creation" << endl;
    fs.chdir("/home/user");
    cout << "createFile('test.txt'): " << (fs.createFile("test.txt") ? "SUCCESS" : "FAILED") << endl;
    cout << "createFile('readme.md'): " << (fs.createFile("readme.md") ? "SUCCESS" : "FAILED") << endl;
    
    cout << "\nContents of /home/user:" << endl;
    vector<string> userContents = fs.listCurrentDirectory();
    for (const string& item : userContents) {
        cout << "- " << item << endl;
    }
    
    // Test 9: Error handling
    cout << "\nTest 9: Error handling" << endl;
    cout << "chdir('/nonexistent'): " << (fs.chdir("/nonexistent") ? "SUCCESS" : "FAILED (as expected)") << endl;
    cout << "mkdir(''): " << (fs.mkdir("") ? "SUCCESS" : "FAILED (as expected)") << endl;
    
    // Test 10: Complex navigation
    cout << "\nTest 10: Complex navigation" << endl;
    fs.chdir("/");
    fs.mkdir("/deep/nested/structure/here");
    cout << "chdir('/deep/nested/structure/here'): " << (fs.chdir("/deep/nested/structure/here") ? "SUCCESS" : "FAILED") << endl;
    cout << "Current directory: " << fs.getCurrentPath() << endl;
    cout << "chdir('../..'): " << (fs.chdir("../..") ? "SUCCESS" : "FAILED") << endl;
    cout << "Current directory: " << fs.getCurrentPath() << endl;
    
    // Test 11: Path existence check
    cout << "\nTest 11: Path existence checks" << endl;
    cout << "pathExists('/home/user'): " << (fs.pathExists("/home/user") ? "EXISTS" : "NOT EXISTS") << endl;
    cout << "pathExists('/nonexistent'): " << (fs.pathExists("/nonexistent") ? "EXISTS" : "NOT EXISTS") << endl;
    
    cout << "\n=== All tests completed ===" << endl;
    
    return 0;
}
