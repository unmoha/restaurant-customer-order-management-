#include <iostream>
#include <string>
#include <map>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <ctime>
#include <cctype>
#include <termios.h>
#include <unistd.h>
using namespace std;

const int MAX_ORDERS = 60;

string getPassword() {
    termios oldt;
    tcgetattr(STDIN_FILENO, &oldt);
    termios newt = oldt;
    newt.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    string password;
    char ch;
    while (read(STDIN_FILENO, &ch, 1) && ch != '\n') {
        if (ch == 127 || ch == 8) { // Handle backspace
            if (!password.empty()) {
                password.pop_back();
                cout << "\b \b"; // Move cursor back, erase character, move back again
            }
        } else {
            password.push_back(ch);
            cout << '*';
        }
    }
    
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    cout << endl;
    return password;
}

struct Order {
    int id;
    string customer;
    string item;
    string category;
    double quantity;
    double total;
    string timestamp;
};

struct Feedback {
    int orderId;
    string message;
    string timestamp;
};

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
    map<int, pair<string, pair<string, double>>> menu;
    int nextId;
    int orderCount;
    const string fileName = "orders.txt";
    const string feedbackFileName = "feedbacks.txt";
    const string passwordFile = "password.txt";
    const string chefPasswordFile = "chef_password.txt";
    const string menuFile = "menu.txt";
    string cashierPassword;
    string chefPassword;

    string currentTime() const {
        time_t t = time(nullptr);
        tm* lt = localtime(&t);
        char buf[20];
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", lt);
        return string(buf);
    }
    
    void loadPasswords() {
        ifstream fin(passwordFile);
        if (fin) {
            getline(fin, cashierPassword);
        } else {
            cashierPassword = "123";
            savePassword();
        }

        ifstream chefFin(chefPasswordFile);
        if (chefFin) {
            getline(chefFin, chefPassword);
        } else {
            chefPassword = "123";
            saveChefPassword();
        }
    }

    void savePassword() const {
        ofstream fout(passwordFile);
        fout << cashierPassword;
    }

    void saveChefPassword() const {
        ofstream fout(chefPasswordFile);
        fout << chefPassword;
    }

    void loadMenu() {
        ifstream min(menuFile);
        if (min) {
            string line;
            while (getline(min, line)) {
                if (line.empty()) continue;
                stringstream ss(line);
                string idStr, name, category, priceStr;
                getline(ss, idStr, ',');
                getline(ss, name, ',');
                getline(ss, category, ',');
                getline(ss, priceStr);
                int id = stoi(idStr);
                double price = stod(priceStr);
                menu[id] = {name, {category, price}};
            }
        } else {
            menu[1] = {"TIBS", {"food", 500.0}};
            menu[2] = {"KITFO", {"food", 600.0}};
            menu[3] = {"DORO WOTIE", {"food", 1050.0}};
            menu[4] = {"MINERAL WATER", {"drink", 30.0}};
            menu[5] = {"BEER", {"drink", 120.0}};
            saveMenu();
        }
    }

    void saveMenu() const {
        ofstream mout(menuFile);
        for (const auto& item : menu) {
            mout << item.first << ',' << item.second.first << ',' 
                 << item.second.second.first << ',' << fixed << setprecision(2) 
                 << item.second.second.second << '\n';
        }
    }

    void loadFromFile() {
        ifstream fin(fileName);
        if (!fin) return;
        string line;
        getline(fin, line);
        int maxId = 1000;
        OrderNode* last = nullptr;
        
        while (getline(fin, line)) {
            if (line.empty()) continue;
            stringstream ss(line);
            Order o;
            getline(ss, line, ','); o.id = stoi(line);
            getline(ss, o.customer, ',');
            getline(ss, o.item, ',');
            getline(ss, o.category, ',');
            getline(ss, line, ','); o.quantity = stod(line);
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
            orderCount++;
        }
        nextId = maxId + 1;
    }

    void saveToFile() const {
        ofstream fout(fileName);
        fout << "ID,Customer,Item,Category,Quantity,Total,Time\n";
        OrderNode* current = ordersHead;
        while (current) {
            const Order& o = current->order;
            fout << o.id << "," << o.customer << "," << o.item << ","
                 << o.category << "," << fixed << setprecision(2) << o.quantity << ","
                 << o.total << "," << o.timestamp << "\n";
            current = current->next;
        }
    }

    bool isValidName(const string& name) const {
        for (char c : name) {
            if (!isalpha(c) && c != ' ') return false;
        }
        return !name.empty();
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

    OrderNode* mergeSort(OrderNode* head, bool (*cmp)(const Order&, const Order&)) {
        if (!head || !head->next) return head;
        
        OrderNode* slow = head;
        OrderNode* fast = head->next;
        while (fast && fast->next) {
            slow = slow->next;
            fast = fast->next->next;
        }
        OrderNode* mid = slow->next;
        slow->next = nullptr;
        
        OrderNode* left = mergeSort(head, cmp);
        OrderNode* right = mergeSort(mid, cmp);
        
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
    OrderManager() : nextId(1001), ordersHead(nullptr), feedbacksHead(nullptr), orderCount(0) {
        loadMenu();

        ofstream create(fileName, ios::app);
        create.close();
        loadFromFile();
        
        ifstream fbCreate(feedbackFileName, ios::app);
        fbCreate.close();
        loadFeedbacks();

        loadPasswords();
    }

    ~OrderManager() {
        OrderNode* currOrder = ordersHead;
        while (currOrder) {
            OrderNode* temp = currOrder;
            currOrder = currOrder->next;
            delete temp;
        }
        
        FeedbackNode* currFeedback = feedbacksHead;
        while (currFeedback) {
            FeedbackNode* temp = currFeedback;
            currFeedback = currFeedback->next;
            delete temp;
        }
    }

    void changePassword(bool isCashier = true) {
        if (isCashier) {
            cout << "Enter current password: ";
            string oldPass = getPassword();
            
            if (oldPass != cashierPassword) {
                cout << "\nIncorrect current password.\n";
                return;
            }
            
            cout << "\nEnter new password: ";
            string newPass = getPassword();
            
            if (newPass.empty()) {
                cout << "\nPassword cannot be empty.\n";
                return;
            }
            
            cashierPassword = newPass;
            savePassword();
            cout << "\nCashier password changed successfully.\n";
        } else {
            cout << "Enter current chef password: ";
            string oldPass = getPassword();
            
            if (oldPass != chefPassword) {
                cout << "\nIncorrect current password.\n";
                return;
            }
            
            cout << "\nEnter new chef password: ";
            string newPass = getPassword();
            
            if (newPass.empty()) {
                cout << "\nPassword cannot be empty.\n";
                return;
            }
            
            chefPassword = newPass;
            saveChefPassword();
            cout << "\nChef password changed successfully.\n";
        }
    }

    bool verifyPassword(const string& input, bool isCashier = true) const {
        return isCashier ? (input == cashierPassword) : (input == chefPassword);
    }

    void displayMenu() const {
        cout << "\n----- FOOD MENU -----\n";
        for (const auto& kv : menu) {
            if (kv.second.second.first == "food") {
                cout << kv.first << ". " << left << setw(20) << kv.second.first
                     << ": " << fixed << setprecision(2) << kv.second.second.second << " birr" << endl;
            }
        }
        
        cout << "\n----- DRINK MENU -----\n";
        for (const auto& kv : menu) {
            if (kv.second.second.first == "drink") {
                cout << kv.first << ". " << left << setw(20) << kv.second.first
                     << ": " << fixed << setprecision(2) << kv.second.second.second << " birr" << endl;
            }
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
                menu[id].second.second = newPrice;
                cout << "Item price updated.\n";
            } else if (changeOption == 3) {
                string newName;
                double newPrice;
                cout << "Enter new name: ";
                getline(cin, newName);
                cout << "Enter new price: ";
                cin >> newPrice;
                cin.ignore();
                menu[id] = {newName, {menu[id].second.first, newPrice}};
                cout << "Item name and price updated.\n";
            } else {
                cout << "Invalid option.\n";
            }
        } else if (option == 2) {
            int newId = menu.rbegin()->first + 1;
            string name, category;
            double price;
            
            cout << "Enter item name: ";
            getline(cin, name);
            
            cout << "Enter category (food/drink): ";
            getline(cin, category);
            
            cout << "Enter price: ";
            cin >> price;
            cin.ignore();
            
            menu[newId] = {name, {category, price}};
            cout << "New menu item added.\n";
        } else {
            cout << "Invalid option.\n";
        }
        saveMenu();
    }
    
    void createOrder() {
        if (orderCount >= MAX_ORDERS) {
            cout << "Maximum order limit (60) reached!\n";
            return;
        }
        
        displayMenu();
        cout << "0. Cancel" << endl;
        int choice;
        cout << "Enter menu number: ";
        cin >> choice;
        cin.ignore();
        if (choice == 0) return;
        if (!menu.count(choice)) {
            cout << "Invalid selection.\n";
            return;
        }

        Order o;
        o.id = nextId++;
        while (true) {
            cout << "Enter customer name: ";
            getline(cin, o.customer);
            if (isValidName(o.customer)) break;
            cout << "Invalid name. Use letters and spaces only.\n";
        }

        o.category = menu.at(choice).second.first;
        o.item = menu.at(choice).first;
        
        if (o.category == "drink") {
            cout << "Enter liters: ";
            cin >> o.quantity;
            cin.ignore();
        } else {
            string qty;
            while (true) {
                cout << "Enter quantity: ";
                getline(cin, qty);
                bool valid = true;
                for (char c : qty) {
                    if (!isdigit(c)) valid = false;
                }
                if (valid && !qty.empty()) {
                    o.quantity = stod(qty);
                    break;
                }
                cout << "Invalid quantity. Use digits only.\n";
            }
        }

        o.total = menu.at(choice).second.second * o.quantity;
        o.timestamp = currentTime();

        OrderNode* newNode = new OrderNode{o, nullptr};
        if (!ordersHead) {
            ordersHead = newNode;
        } else {
            OrderNode* current = ordersHead;
            while (current->next) {
                current = current->next;
            }
            current->next = newNode;
        }
        
        orderCount++;
        saveToFile();
        cout << "Order ID " << o.id << " created at " << o.timestamp << ". Total: "
             << fixed << setprecision(2) << o.total << " birr.\n";
    }

    void listOrders() const {
        if (!ordersHead) {
            cout << "\nNo orders to display.\n";
            return;
        }
        cout << "\n------ Order List ------\n";
        cout << left << setw(6) << "ID" << setw(20) << "Customer"
             << setw(20) << "Item" << setw(8) << "Qty"
             << setw(10) << "Total" << "Time" << endl;
        cout << string(70, '-') << endl;
        
        OrderNode* current = ordersHead;
        while (current) {
            const Order& o = current->order;
            cout << left << setw(6) << o.id
                 << setw(20) << o.customer
                 << setw(20) << o.item
                 << setw(8) << fixed << setprecision(2) << o.quantity
                 << setw(10) << o.total
                 << o.timestamp << endl;
            current = current->next;
        }
    }

    void updateOrderById() {
        int id;
        cout << "Enter Order ID to update: ";
        cin >> id;
        cin.ignore();
        
        OrderNode* current = ordersHead;
        while (current) {
            if (current->order.id == id) {
                double oldTotal = current->order.total;
                
                if (current->order.category == "drink") {
                    cout << "Enter new liters: ";
                    cin >> current->order.quantity;
                    cin.ignore();
                } else {
                    string qty;
                    while (true) {
                        cout << "Enter new quantity: ";
                        getline(cin, qty);
                        bool valid = true;
                        for (char c : qty) {
                            if (!isdigit(c)) valid = false;
                        }
                        if (valid && !qty.empty()) {
                            current->order.quantity = stod(qty);
                            break;
                        }
                        cout << "Invalid quantity. Use digits only.\n";
                    }
                }
                
                for (const auto& item : menu) {
                    if (item.second.first == current->order.item) {
                        current->order.total = item.second.second.second * current->order.quantity;
                        break;
                    }
                }
                
                double diff = current->order.total - oldTotal;
                saveToFile();
                cout << "Order " << id << " updated. New total: "
                     << fixed << setprecision(2) << current->order.total << " birr.\n";
                if (diff != 0) {
                    cout << "Price changed by: " << fixed << setprecision(2) << abs(diff) 
                         << " birr (" << (diff > 0 ? "+" : "") << diff << ")\n";
                }
                return;
            }
            current = current->next;
        }
        cout << "Order ID not found.\n";
    } 
    
    void deleteOrderById() {
        int id;
        cout << "Enter Order ID to delete: ";
        cin >> id;
        cin.ignore();
        
        OrderNode* current = ordersHead;
        while (current) {
            if (current->order.id == id) {
                current->order.item = "[DELETED]";
                current->order.quantity = 0;
                current->order.total = 0.0;
                saveToFile();
                orderCount--;
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
        cin.ignore();
        
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
        saveToFile();
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
                     << setw(10) << fixed << setprecision(2) << item.second 
                     << setw(15) << itemRevenues[item.first] << endl;
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
        cin.ignore();

        bool orderExists = false;
        OrderNode* current = ordersHead;
        while (current) {
            if (current->order.id == orderId && current->order.item != "[DELETED]") {
                orderExists = true;
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
        
        saveFeedback(fb);
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
        map<string, double> itemCount;
        OrderNode* current = ordersHead;
        while (current) {
            if (current->order.item != "[DELETED]" && current->order.category == "food") {
                itemCount[current->order.item] += current->order.quantity;
            }
            current = current->next;
        }
        if (itemCount.empty()) {
            cout << "** Most Popular Food: (No valid orders) **\n";
            return;
        }
        auto best = max_element(itemCount.begin(), itemCount.end(),
                [](const pair<string, double>& a, const pair<string, double>& b) {
                    return a.second < b.second;
                });
        cout << "** Most Popular Food: " << best->first << " (Sold " << best->second << " units) **\n";
    }
}; 

void showCashierMenu() {
    cout << "\n===== Cashier Menu =====\n";
    cout << "1. Create Order\n";
    cout << "2. List Orders\n";
    cout << "3. Update Order\n";
    cout << "4. Delete Order\n";
    cout << "5. Search Order\n";
    cout << "6. Sort Orders\n";
    cout << "7. Update Menu\n";
    cout << "8. Daily Sales Report\n";
    cout << "9. Change Password\n";
    cout << "0. Exit\n";
    cout << "Choose: ";
}

void showCustomerMenu() {
    cout << "\n=== Customer Menu ===\n";
    cout << "1. Create Order\n";
    cout << "2. Search Order by ID\n";
    cout << "3. Update Order by ID\n";
    cout << "4. Delete Order by ID\n";
    cout << "5. Submit Feedback\n";
    cout << "0. Back\n";
    cout << "Choose: ";
}

void showChefMenu() {
    cout << "\n=== Chef Menu ===\n";
    cout << "1. View Feedbacks\n";
    cout << "2. List Orders\n";
    cout << "3. Change Password\n";
    cout << "0. Back\n";
    cout << "Choose: ";
}

int main() {
    OrderManager sharedOM;
    while (true) {
        cout << "Select role:\n1. Customer\n2. Cashier\n3. Chef\n0. Exit\nChoice: ";
        int role;
        cin >> role;
        cin.ignore();

        if (role == 0) break;
        if (role == 1) {
            while (true) {
                sharedOM.displayFamousFood();
                showCustomerMenu();
                int c;
                cin >> c;
                cin.ignore();
                if (c == 0) break;
                switch(c) {
                    case 1: sharedOM.createOrder(); break;
                    case 2: sharedOM.searchOrder(); break;
                    case 3: sharedOM.updateOrderById(); break;
                    case 4: sharedOM.deleteOrderById(); break;
                    case 5: sharedOM.submitFeedback(); break;
                    default: cout << "Invalid choice.\n";
                }
            }
        } 
        else if (role == 2) {
            cout << "Enter cashier password: ";
            string pwd = getPassword();
            if (!sharedOM.verifyPassword(pwd)) {
                cout << "\nWrong password.\n";
                continue;
            }
            while (true) {
                sharedOM.displayFamousFood();
                showCashierMenu();
                int choice;
                cin >> choice;
                cin.ignore();
                if (choice == 0) break;
                switch (choice) {
                    case 1: sharedOM.createOrder(); break;
                    case 2: sharedOM.listOrders(); break;
                    case 3: sharedOM.updateOrderById(); break;
                    case 4: sharedOM.deleteOrderById(); break;
                    case 5: sharedOM.searchOrder(); break;
                    case 6: sharedOM.sortOrders(); break;
                    case 7: sharedOM.updateMenu(); break;
                    case 8: sharedOM.generateDailyReport(); break;
                    case 9: sharedOM.changePassword(); break;
                    default: cout << "Invalid choice.\n";
                }
            }
        }
        else if (role == 3) {
            cout << "Enter chef password: ";
            string pwd = getPassword();
            if (!sharedOM.verifyPassword(pwd, false)) {
                cout << "\nWrong password.\n";
                continue;
            }
            while (true) {
                showChefMenu();
                int choice;
                cin >> choice;
                cin.ignore();
                if (choice == 0) break;
                switch (choice) {
                    case 1: sharedOM.viewFeedbacks(); break;
                    case 2: sharedOM.listOrders(); break;
                    case 3: sharedOM.changePassword(false); break;
                    default: cout << "Invalid choice.\n";
                }
            }
        }
        else {
            cout << "Invalid role selected.\n";
        }
    }
    cout << "Goodbye!\n";
    return 0;
}
