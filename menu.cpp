#include <iostream>
#include <string>
#include <map>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <ctime>
#include <cctype>
using namespace std;

struct Order {
    int id;
    string customer;
    string item;
    int quantity;
    double total;
    string timestamp;
};

struct Feedback {
    int orderId;
    string message;
    string timestamp;
};

// Node structures for linked lists
struct OrderNode {
    Order order;
    OrderNode* next;
};

struct FeedbackNode {
    Feedback feedback;
    FeedbackNode* next;
};

class OrderManager {
private:
    OrderNode* ordersHead;
    FeedbackNode* feedbacksHead;
    map<int, pair<string,double>> menu;
    int nextId;
    const string fileName = "orders.txt";
    const string feedbackFileName = "feedbacks.txt";
    const string passwordFile = "password.txt";
    const string menuFile = "menu.txt";  // New menu file
    string cashierPassword;

    string currentTime() const {
        time_t t = time(nullptr);
        tm* lt = localtime(&t);
        char buf[20];
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", lt);
        return string(buf);
    }
    
    void loadPassword() {
        ifstream fin(passwordFile);
        if (fin) {
            getline(fin, cashierPassword);
        } else {
            cashierPassword = "123"; // default password
            savePassword();
        }
    }

    void savePassword() const {
        ofstream fout(passwordFile);
        fout << cashierPassword;
    }

    void loadMenu() {  // New function to load menu
        ifstream min(menuFile);
        if (min) {
            string line;
            while (getline(min, line)) {
                if (line.empty()) continue;
                stringstream ss(line);
                string idStr, name, priceStr;
                getline(ss, idStr, ',');
                getline(ss, name, ',');
                getline(ss, priceStr);
                int id = stoi(idStr);
                double price = stod(priceStr);
                menu[id] = {name, price};
            }
        } else {
            // Default menu if file doesn't exist
            menu[1] = {"TIBS", 500.0};
            menu[2] = {"KITFO", 600.0};
            menu[3] = {"DORO WOTIE", 1050.0};
            menu[4] = {"KUANTA FERFER", 650.0};
            menu[5] = {"ALCHA KEKLE", 530.0};
            saveMenu();  // Save default menu
        }
    }

    void saveMenu() const {  // New function to save menu
        ofstream mout(menuFile);
        for (const auto& item : menu) {
            mout << item.first << ',' << item.second.first << ',' 
                 << fixed << setprecision(2) << item.second.second << '\n';
        }
    }

    void loadFromFile() {
        ifstream fin(fileName);
        if (!fin) return;
        string line;
        getline(fin, line); // Skip header
        int maxId = 1000;
        OrderNode* last = nullptr;
        
        while (getline(fin, line)) {
            if (line.empty()) continue;
            stringstream ss(line);
            Order o;
            getline(ss, line, ','); o.id = stoi(line);
            getline(ss, o.customer, ',');
            getline(ss, o.item, ',');
            getline(ss, line, ','); o.quantity = stoi(line);
            getline(ss, line, ','); o.total = stod(line);
            getline(ss, o.timestamp);
            
            OrderNode* newNode = new OrderNode{o, nullptr};
            if (!ordersHead) {
                ordersHead = newNode;
                last = newNode;
            } else {
                last->next = newNode;
                last = newNode;
            }
            maxId = max(maxId, o.id);
        }
        nextId = maxId + 1;
    }

    void saveToFile() const {
        ofstream fout(fileName);
        fout << "ID,Customer,Item,Quantity,Total,Time\n";
        OrderNode* current = ordersHead;
        while (current) {
            const Order& o = current->order;
            fout << o.id << "," << o.customer << "," << o.item << ","
                 << o.quantity << "," << fixed << setprecision(2) << o.total
                 << "," << o.timestamp << "\n";
            current = current->next;
        }
    }

    bool isValidName(const string& name) const {
        for (char c : name) {
            if (!isalpha(c) && c != ' ') return false;
        }
        return !name.empty();
    }

    bool isValidQuantity(const string& input) const {
        for (char c : input) {
            if (!isdigit(c)) return false;
        }
        return !input.empty();
    }

    void loadFeedbacks() {
        ifstream fin(feedbackFileName);
        if (!fin) return;
        string line;
        FeedbackNode* last = nullptr;
        
        while (getline(fin, line)) {
            if (line.empty()) continue;
            stringstream ss(line);
            Feedback fb;
            getline(ss, line, ';'); fb.orderId = stoi(line);
            getline(ss, fb.timestamp, ';');
            getline(ss, fb.message);
            
            FeedbackNode* newNode = new FeedbackNode{fb, nullptr};
            if (!feedbacksHead) {
                feedbacksHead = newNode;
                last = newNode;
            } else {
                last->next = newNode;
                last = newNode;
            }
        }
    }

    void saveFeedback(const Feedback& fb) const {
        ofstream fout(feedbackFileName, ios::app);
        fout << fb.orderId << ';' << fb.timestamp << ';' << fb.message << '\n';
    }

    // Merge Sort for linked list
    OrderNode* mergeSort(OrderNode* head, bool (*cmp)(const Order&, const Order&)) {
        if (!head || !head->next) return head;
        
        // Split list
        OrderNode* slow = head;
        OrderNode* fast = head->next;
        while (fast && fast->next) {
            slow = slow->next;
            fast = fast->next->next;
        }
        OrderNode* mid = slow->next;
        slow->next = nullptr;
        
        // Recursively sort
        OrderNode* left = mergeSort(head, cmp);
        OrderNode* right = mergeSort(mid, cmp);
        
        // Merge sorted lists
        OrderNode dummy;
        OrderNode* tail = &dummy;
        
        while (left && right) {
            if (cmp(left->order, right->order)) {
                tail->next = left;
                left = left->next;
            } else {
                tail->next = right;
                right = right->next;
            }
            tail = tail->next;
        }
        
        tail->next = (left) ? left : right;
        return dummy.next;
    }

    static bool compareByTime(const Order& a, const Order& b) {
        return a.timestamp < b.timestamp;
    }

    public:
    OrderManager() : nextId(1001), ordersHead(nullptr), feedbacksHead(nullptr) {
        // Initialize menu from file
        loadMenu();

        ofstream create(fileName, ios::app);
        create.close();
        loadFromFile();
        
        // Load feedbacks
        ifstream fbCreate(feedbackFileName, ios::app);
        fbCreate.close();
        loadFeedbacks();

        // Load password
        loadPassword();
    }

    ~OrderManager() {
        // Cleanup orders
        OrderNode* currOrder = ordersHead;
        while (currOrder) {
            OrderNode* temp = currOrder;
            currOrder = currOrder->next;
            delete temp;
        }
        
        // Cleanup feedbacks
        FeedbackNode* currFeedback = feedbacksHead;
        while (currFeedback) {
            FeedbackNode* temp = currFeedback;
            currFeedback = currFeedback->next;
            delete temp;
        }
    }

    void changePassword() {
        string oldPass, newPass;
        cout << "Enter current password: ";
        getline(cin, oldPass);
        
        if (oldPass != cashierPassword) {
            cout << "Incorrect current password.\n";
            return;
        }
        
        cout << "Enter new password: ";
        getline(cin, newPass);
        
        if (newPass.empty()) {
            cout << "Password cannot be empty.\n";
            return;
        }
        
        cashierPassword = newPass;
        savePassword();
        cout << "Password changed successfully.\n";
    }

    bool verifyPassword(const string& input) const {
        return input == cashierPassword;
    }

    void displayMenu() const {
        cout << "\n----- SPECIAL MENU -----\n";
        for (const auto& kv : menu) {
            cout << kv.first << ". " << left << setw(15) << kv.second.first
                 << ": " << fixed << setprecision(2) << kv.second.second << " birr" << endl;
        }
        cout << "------------------------\n";
    }

    void updateMenu() {
        int option;
        cout << "\n1. Update Existing Item\n2. Add New Item\nChoose: ";
        cin >> option;
        cin.ignore();

        if (option == 1) {
            int id;
            cout << "Enter item ID to update: ";
            cin >> id;
            cin.ignore();
            if (!menu.count(id)) {
                cout << "Item ID not found.\n";
                return;
            }
            cout << "Update:\n1. Name only\n2. Price only\n3. Both\nChoose: ";
            int changeOption;
            cin >> changeOption;
            cin.ignore();

            if (changeOption == 1) {
                string newName;
                cout << "Enter new name: ";
                getline(cin, newName);
                menu[id].first = newName;
                cout << "Item name updated.\n";
            } else if (changeOption == 2) {
                double newPrice;
                cout << "Enter new price: ";
                cin >> newPrice;
                cin.ignore();
                menu[id].second = newPrice;
                cout << "Item price updated.\n";
            } else if (changeOption == 3) {
                string newName;
                double newPrice;
                cout << "Enter new name: ";
                getline(cin, newName);
                cout << "Enter new price: ";
                cin >> newPrice;
                cin.ignore();
                menu[id] = {newName, newPrice};
                cout << "Item name and price updated.\n";
            } else {
                cout << "Invalid option.\n";
            }
        } else if (option == 2) {
            int newId = menu.rbegin()->first + 1;
            string name;
            double price;
            cout << "Enter item name: ";
            getline(cin, name);
            cout << "Enter price: ";
            cin >> price;
            cin.ignore();
            menu[newId] = {name, price};
            cout << "New menu item added.\n";
        } else {
            cout << "Invalid option.\n";
        }
        saveMenu(); // Save menu changes to file
    }
     void deleteOrderById() {
        int id;
        cout << "Enter Order ID to delete: ";
        cin >> id;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        
        OrderNode* current = ordersHead;
        while (current) {
            if (current->order.id == id) {
                current->order.item = "[DELETED]";
                current->order.quantity = 0;
                current->order.total = 0.0;
                saveToFile(); // Save deletion immediately
                cout << "Order " << id << " marked as deleted.\n";
                return;
            }
            current = current->next;
        }
        cout << "Order ID not found.\n";
    }

    void searchOrder() const {
        int id;
        cout << "Enter Order ID to search: ";
        cin >> id;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        
        OrderNode* current = ordersHead;
        while (current) {
            if (current->order.id == id) {
                const Order& o = current->order;
                cout << "Found Order: " << o.customer << " ordered "
                     << o.item << " x" << o.quantity << " at "
                     << o.timestamp << ". Total: " << fixed << setprecision(2)
                     << o.total << " birr.\n";
                return;
            }
            current = current->next;
        }
        cout << "Order ID not found.\n";
    }

    void sortOrders() {
        ordersHead = mergeSort(ordersHead, compareByTime);
        cout << "Orders sorted by time.\n";
        listOrders();
        saveToFile(); // Save sorted orders immediately
    }

    void generateDailyReport() const {
        string today = currentTime().substr(0, 10);
        double totalRevenue = 0.0;
        int totalOrders = 0;
        map<string, int> itemQuantities;
        map<string, double> itemRevenues;

        OrderNode* current = ordersHead;
        while (current) {
            const Order& o = current->order;
            if (o.timestamp.substr(0, 10) == today && o.item != "[DELETED]") {
                totalOrders++;
                totalRevenue += o.total;
                itemQuantities[o.item] += o.quantity;
                itemRevenues[o.item] += o.total;
            }
            current = current->next;
        }

        cout << "\n====== Daily Sales Report ======\n";
        cout << "Date: " << today << "\n";
        cout << "Total Orders: " << totalOrders << "\n";
        cout << "Total Revenue: " << fixed << setprecision(2) << totalRevenue << " birr\n\n";
        
        if (!itemQuantities.empty()) {
            cout << "Item-wise Sales:\n";
            cout << left << setw(20) << "Item" << setw(10) << "Quantity" 
                 << setw(15) << "Revenue (birr)" << endl;
            cout << string(45, '-') << endl;
            
            for (const auto& item : itemQuantities) {
                cout << left << setw(20) << item.first 
                     << setw(10) << item.second 
                     << setw(15) << fixed << setprecision(2) << itemRevenues[item.first] << endl;
            }
        } else {
            cout << "No sales today.\n";
        }
        cout << "===============================\n";
    }
   void submitFeedback() {
        int orderId;
        cout << "Enter your Order ID: ";
        cin >> orderId;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        bool orderExists = false;
        OrderNode* current = ordersHead;
        while (current) {
            if (current->order.id == orderId && current->order.item != "[DELETED]") {
                orderExists = true
                break;
            }
            current = current->next;
        }

        if (!orderExists) {
            cout << "Order ID not found or invalid.\n";
            return;
        }

        string message;
        cout << "Enter your feedback: ";
        getline(cin, message);

        if (message.empty()) {
            cout << "Feedback cannot be empty.\n";
            return;
        }

        Feedback fb;
        fb.orderId = orderId;
        fb.message = message;
        fb.timestamp = currentTime();

        // Add to feedbacks linked list
        FeedbackNode* newNode = new FeedbackNode{fb, nullptr};
        if (!feedbacksHead) {
            feedbacksHead = newNode;
        } else {
            FeedbackNode* currentFB = feedbacksHead;
            while (currentFB->next) {
                currentFB = currentFB->next;
            }
            currentFB->next = newNode;
        }
        
        saveFeedback(fb); // Save feedback to file
        cout << "Thank you for your feedback!\n";
    }

    void viewFeedbacks() const {
        if (!feedbacksHead) {
            cout << "No feedback available.\n";
            return;
        }

        cout << "\n====== Customer Feedbacks ======\n";
        cout << left << setw(10) << "Order ID" << setw(20) << "Time" << "Feedback" << endl;
        cout << string(70, '-') << endl;
        
        FeedbackNode* current = feedbacksHead;
        while (current) {
            const Feedback& fb = current->feedback;
            cout << left << setw(10) << fb.orderId 
                 << setw(20) << fb.timestamp 
                 << fb.message << endl;
            current = current->next;
        }
        cout << "===============================\n";
    }

    void displayFamousFood() const {
        if (!ordersHead) {
            cout << "** Most Popular Food: (No orders yet) **\n";
            return;
        }
        map<string, int> itemCount;
        OrderNode* current = ordersHead;
        while (current) {
            if (current->order.item != "[DELETED]") {
                itemCount[current->order.item] += current->order.quantity;
            }
            current = current->next;
        }
        if (itemCount.empty()) {
            cout << "** Most Popular Food: (No valid orders) **\n";
            return;
        }
        auto best = max_element(itemCount.begin(), itemCount.end(),
                [](const pair<string, int>& a, const pair<string, int>& b) {
                    return a.second < b.second;
                });
        cout << "** Most Popular Food: " << best->first << " (Ordered " << best->second << " times) **\n";
    }
}; 

void showMainMenu() {
    cout << "\n===== Selam Ethiopian Restaurant =====\n";
    cout << "1. Create Order\n";
    cout << "2. List Orders\n";
    cout << "3. Update Order\n";
    cout << "4. Delete Order\n";
    cout << "5. Search Order\n";
    cout << "6. Sort Orders\n";
    cout << "7. Update Menu\n";
    cout << "8. Daily Sales Report\n";
    cout << "9. View Feedbacks\n";
    cout << "10. Change Password\n";
    cout << "0. Exit\n";
    cout << "Choose: ";
}

int main() {
    OrderManager sharedOM;
    while (true) {
        cout << "Select role:\n1. Customer\n2. Cashier\n0. Exit\nChoice: ";
        int role;
        cin >> role;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        if (role == 0) break;
        if (role == 1) {
            while (true) {
                cout << "\n=== Customer Menu ===\n";
                sharedOM.displayFamousFood();
                cout << "1. Create Order\n";
                cout << "2. Search Order by ID\n";
                cout << "3. Update Order by ID\n";
                cout << "4. Delete Order by ID\n";
                cout << "5. Submit Feedback\n";
                cout << "0. Back\n";
                cout << "Choose: ";
                int c;
                cin >> c;
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                if (c == 0) break;
                else if (c == 1) sharedOM.createOrder();
                else if (c == 2) sharedOM.searchOrder();
                else if (c == 3) sharedOM.updateOrderById();
                else if (c == 4) sharedOM.deleteOrderById();
                else if (c == 5) sharedOM.submitFeedback();
                else cout << "Invalid choice.\n";
            }
        } else if (role == 2) {
            string pwd;
            cout << "Enter password: ";
            getline(cin, pwd);
            if (!sharedOM.verifyPassword(pwd)) {
                cout << "Wrong password.\n";
                continue;
            }
            while (true) {
                sharedOM.displayFamousFood();
                showMainMenu();
                int choice;
                cin >> choice;
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                switch (choice) {
                    case 0: cout << "Returning to role selection...\n"; goto role_menu;
                    case 1: sharedOM.createOrder(); break;
                    case 2: sharedOM.listOrders();  break;
                    case 3: sharedOM.updateOrderById(); break;
                    case 4: sharedOM.deleteOrderById(); break;
                    case 5: sharedOM.searchOrder(); break;
                    case 6: sharedOM.sortOrders();  break;
                    case 7: sharedOM.updateMenu();   break;
                    case 8: sharedOM.generateDailyReport(); break;
                    case 9: sharedOM.viewFeedbacks(); break;
                    case 10: sharedOM.changePassword(); break;
                    default: cout << "Invalid choice.\n";
                }
            }
        } else {
            cout << "Invalid role selected.\n";
        }
role_menu:;
    }
    cout << "Goodbye!\n";
    return 0;
}
