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
