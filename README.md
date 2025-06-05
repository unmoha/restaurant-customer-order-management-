Restaurant Order Management System
Overview
This C++ program is a comprehensive Restaurant Order Management System designed for a small to medium-sized restaurant. It provides three distinct user roles with different functionalities:

Customer: Can place orders, search/update/delete their orders, and submit feedback

Cashier: Full order management capabilities plus menu management and sales reporting

Chef: Can view customer feedbacks and monitor orders

The system features file-based data persistence, password protection for staff roles, and various order management functionalities.

Features
Core Functionalities
Order Management:

Create new orders with customer details

View, update, and delete existing orders

Search orders by ID

Sort orders by timestamp

Menu Management:

View food and drink menus

Add new menu items

Update existing items (name, price, or both)

Reporting:

Daily sales reports with revenue breakdown

Most popular food item tracking

Feedback System:

Customers can submit feedback

Chef can view all feedbacks

Technical Features
File-based data persistence (orders, menu, feedbacks, passwords)

Password protection with hidden input

Input validation

Merge sort algorithm for order sorting

Linked list data structure for order storage

User Roles
1. Customer
Place new orders

Search for their orders

Update or cancel their orders

Submit feedback

2. Cashier (Password Protected)
All customer functionalities

View all orders

Sort orders

Update restaurant menu

Generate daily sales reports

Change cashier password

3. Chef (Password Protected)
View customer feedbacks

View all orders

Change chef password

Data Storage
The system stores data in several text files:

orders.txt - All order records

feedbacks.txt - Customer feedbacks

menu.txt - Restaurant menu items

password.txt - Cashier password

chef_password.txt - Chef password

How to Use
Compile the program using a C++ compiler (g++ recommended)

Run the executable

Select your role (Customer, Cashier, or Chef)

For staff roles, enter the correct password (default is "123")

Navigate through the menu options to perform actions

Default Credentials
Cashier password: 123

Chef password: 123

Requirements
C++11 or later

Unix-like system (for termios.h header)

Basic understanding of restaurant operations

Notes
The system is designed for single-computer use in a restaurant setting

All data is stored in text files in the same directory as the executable

The system has a maximum order limit of 60 orders

Future Improvements
Add database support for better scalability

Implement a graphical user interface

Add more reporting features

Include table/reservation management

License
This code is provided as-is for educational purposes. Feel free to modify and use it according to your needs.

