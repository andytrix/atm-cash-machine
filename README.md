# ATM Cash Machine

A feature-rich ATM simulator built with Qt, Node.js, and MySQL, supporting authentication, transactions, and card management—bringing a real banking experience to the digital world.

# Features

- **User Authentication:** Login with a PIN-based system secured using bcrypt hashing
- **Banking Transactions:** Deposit, withdraw, and transfer money
- **Card Management:** Multiple card types with status tracking (active, locked, inactive)
- **Card Types:** Supports debit cards, credit cards, and dual cards (credit/debit)
- **Database Integration:** MySQL-based backend with structured data storage
- **Multi-Language Support:** Available in Finnish, Swedish, and English
- **Profile Picture Upload:** Users can add their own profile picture
- **PIN Code Change:** Users can securely update their PIN code
- **Auto Logout Timer:** 30 seconds timeout from Main Menu, 10 seconds timeout in other views
- **Realistic Interface:** Built with Qt for an authentic ATM experience

# Technology Stack

Frontend: Qt Creator (C++)  
Backend: Node.js with Express.js  
Database: MySQL

# Installation & Setup

**Prerequisites**

Ensure you have the following installed:  
- Qt Creator (for UI development)
- Node.js & npm (for backend)
- MySQL Server (for database storage)

**1. Clone the Repository**

```bash
git clone https://github.com/andytrix/atm-cash-machine.git  
cd atm-cash-machine
```
**2. Setup the Database**

- Start your MySQL server.  
- Import the database schema:

```bash
mysql -u root -p < db_dump.sql
```
**3. Start the Backend Server**

```bash
cd backend  
npm install
npm install multer  
node server.js  
```
**4. Run the Qt Application**

- Open Qt Creator.
- Load the project file (.pro).
- Build and run the application.

# Database Schema

The system is structured into multiple tables:

- **customer:** Stores user details, including first name, last name, profile picture, and account creation date.
- **card:** Holds card-specific data, including debit, credit, and dual card types, PIN codes, status (active, locked, inactive), and expiry date.
- **card_account:** Links cards to accounts and defines whether the link is debit or credit.
- **account:** Tracks account balances, credit limits, and account creation date.
- **transaction:** Logs all ATM transactions, including withdrawals and deposits.

### **Entity-Relationship (ER) Diagram**

Below is the ER diagram representing the database structure:  

<img width="886" alt="ER-kaavio-v1 3" src="https://github.com/user-attachments/assets/48b6d0eb-0a5a-4d1b-9ebf-ad9c83fb25a6" />

### **Deployment Diagram**

Shows how software components are distributed across different hardware and network nodes:  

![Deployment_Diagram](https://github.com/user-attachments/assets/9e3e1934-8faa-452b-9d9a-d5c3147ad3fd)

### **Class Diagram**

Illustrates the relationships between key classes in the project:  

![Luokkakaavio](https://github.com/user-attachments/assets/28c764c3-62d0-4250-a040-1e21812b6937)


