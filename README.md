# ATM Cash Machine

A feature-rich ATM simulator built with Qt, Node.js, and MySQL, supporting authentication, transactions, and card management—bringing a real banking experience to the digital world.

# Features

- User Authentication: Login with a PIN-based system secured using bcrypt hashing
- Banking Transactions: Deposit, withdraw, and transfer money
- Card Management: Multiple card types with status tracking (active, locked, inactive)
- Card Types: Supports debit cards, credit cards, and dual cards (credit/debit)
- Database Integration: MySQL-based backend with structured data storage
- Multi-Language Support: Available in Finnish, Swedish, and English
- Profile Picture Upload: Users can add their own profile picture
- PIN Code Change: Users can securely update their PIN code
- Auto Logout Timer: 30 seconds timeout from Main Menu, 10 seconds timeout in other views
- Realistic Interface: Built with Qt for an authentic ATM experience

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
```
```bash
cd atm-cash-machine
```
**2. Setup the Database**

- Start your MySQL server.  
- Import the database schema:

```bash
mysql -u root -p < db_dump.sql
```
