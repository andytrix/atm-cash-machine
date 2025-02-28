# ATM Cash Machine

Experience a full-featured ATM simulator powered by Qt, Node.js, and MySQL. With secure authentication, dynamic transactions, and comprehensive card management, it brings the authentic feel of real-world banking right to your screen.

<p align="left">
  <img src="https://github.com/user-attachments/assets/049496ca-0f47-4c48-813c-28f51802c0b7" 
       alt="ATM 1" width="480" style="margin-right: 10px;" />
  <img src="https://github.com/user-attachments/assets/2e31eb09-a4d5-45df-8d2d-3d76bdf1618b"
       alt="ATM 2" width="480" />
</p>

# Contents

- [Introduction](#atm-cash-machine)
- [Features](#features)
- [Technology Stack](#technology-stack)
- [Installation & Setup](#installation--setup)
- [Creating a User and Linking a Card](#creating-a-user-and-linking-a-card)
- [Directory Structure](#directory-structure)
- [System, Database & Class Architecture](#system-database--class-architecture)
  - [Database Schema](#database-schema)
  - [Entity-Relationship (ER) Diagram](#entity-relationship-er-diagram)
  - [Class Diagram](#class-diagram)
  - [Deployment Diagram](#deployment-diagram)

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

Make sure you have the following installed on your system:

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
- (Optional) Create the bankuser account if it does not exist yet. Log into MySQL (e.g., mysql -u root -p) and run:
- 
 ```bash
CREATE USER 'bankuser'@'localhost' IDENTIFIED BY 'bankuser';
GRANT ALL PRIVILEGES ON *.* TO 'bankuser'@'localhost';
FLUSH PRIVILEGES;
```

- Import the database schema from the project directory (where db_dump.sql is located):

```bash
mysql -u root -p < db_dump.sql
```

**3. Configure Environment Variables**

Use the .env file as a template or create a .env file in the backend directory with the following content:

```bash
DB_HOST=127.0.0.1
DB_USER=bankuser
DB_PASS=bankuser
DB_NAME=bank-automat
MY_TOKEN=supersecret
DB_PORT=3306
```

**4. Start the Backend Server**

```bash
cd backend  
npm install
npm start  
```
By default, the server will listen on http://localhost:3000.

**5. Run the Qt Application**

- Open Qt Creator.
- Load the project file (.pro).
- Build and run the application.

**6. Testing**
You can test the backend endpoints using a tool like Postman or your web browser (e.g. http://localhost:3000/account).
If your login route is something like POST /login, you’ll likely need to provide a card number and a PIN (which is hashed in the database).
On successful login, the server should return a token. Copy that token from the response.
On subsequent requests to protected endpoints, include the token in an Authorization: Bearer <token> header.

# Creating a User and Linking a Card

**Creating a User with a Single (Debit) Card**

Insert a new customer:

```bash
INSERT INTO customer (idcustomer, fname, lname, thumbnail, created_at)
VALUES (1, 'John', 'Doe', 'example.png', NOW());
```
- idcustomer is the primary key.
- fname and lname are the user’s first and last names.
- thumbnail can be any image filename or NULL if you’re not using profile pictures.

Insert a new card:

```bash
INSERT INTO card (idcard, idcustomer, pin, cardtype, status, created_at, expiry_date)
VALUES (
    1,
    1,
    '$2a$10$Kf86Rkz9gt21rvkgX4/t6eWSaaH65ZECxywqQkIvouezrur9ZYZNC',  -- hashed PIN for "1234"
    'single',  -- indicates this is a single-type card (debit or credit)
    'active',
    NOW(),
    '2030-12-31 00:00:00'
);
```

- cardtype='single' means this card is either debit or credit. We’ll link it as debit in the next steps.
- status='active' indicates the card is ready for use.

Insert a debit account:

```bash
INSERT INTO account (idaccount, idcustomer, type, credit_balance, credit_limit, debit_balance, created_at)
VALUES (
    1,
    1,
    'debit',
    0.00,
    0.00,
    1000.00,  -- e.g., an initial debit balance
    NOW()
);
```

- type='debit' marks this account as a debit account.
- debit_balance is the current amount of money available.

Link the card to the account:

```bash
INSERT INTO card_account (idcard_account, idcard, idaccount, type)
VALUES (
    1,
    1,
    1,
    'debit'
);
```

- This tells the system that card 1 is associated with account 1 in a debit capacity.

At this point, you have:

- One customer (idcustomer=1)
- One card (idcard=1)
- One debit account (idaccount=1)
- A card_account link specifying that card #1 is a debit card for account #1.

**Creating a User with a Dual Card (Debit + Credit)**

Insert a customer (If you’re using the same customer as above, you can skip re-inserting them. Otherwise, insert a new one with a unique idcustomer):

```bash
INSERT INTO customer (idcustomer, fname, lname, thumbnail, created_at)
VALUES (<unique idcustomer>, 'Jane', 'Doe', 'example.png', NOW());
```
Insert a new card:

```bash
INSERT INTO card (idcard, idcustomer, pin, cardtype, status, created_at, expiry_date)
VALUES (
    2,
    1,
    '$2a$10$Kf86Rkz9gt21rvkgX4/t6eWSaaH65ZECxywqQkIvouezrur9ZYZNC',  -- hashed PIN for "1234"
    'dual',   -- indicates a dual card
    'active',
    NOW(),
    '2030-12-31 00:00:00'
);
```
- cardtype='dual' means this single card can handle both debit and credit.

Insert a credit account:

```bash
INSERT INTO account (idaccount, idcustomer, type, credit_balance, credit_limit, debit_balance, created_at)
VALUES (
    2,
    1,
    'credit',
    0.00,
    2000.00,  -- for example, a 2000 credit limit
    0.00,
    NOW()
);
```

Insert a debit account:

```bash
INSERT INTO account (idaccount, idcustomer, type, credit_balance, credit_limit, debit_balance, created_at)
VALUES (
    3,
    1,
    'debit',
    0.00,
    0.00,
    1500.00,  -- e.g., an initial debit balance
    NOW()
);
```
Link the card to both accounts:

```bash
INSERT INTO card_account (idcard_account, idcard, idaccount, type)
VALUES 
    (2, 2, 3, 'debit'),
    (3, 2, 2, 'credit');
```

Here we create two links:

- idcard_account=2 for the debit account (account #3)
- idcard_account=3 for the credit account (account #2)

# Directory Structure

``` ATM/
├── LICENSE                    # The license file specifying terms for using this project.
├── README.md                  # The main README file explaining the project.
├── .gitignore                 # Specifies files and directories to be ignored by Git.
├── db_dump.sql                # SQL file containing the initial database schema.
├── ER-diagram.png             # Entity-Relationship (ER) diagram visualizing the database structure.
├── Class_Diagram.png          # Class diagram representing object-oriented relationships in the project.
├── Deployment_Diagram.png     # Deployment diagram illustrating the system's deployment architecture.
├── backend/                   
│   ├── bin/                   # Holds executables or compiled files related to backend.
│   ├── www/                   # Contains web-related resources for the backend.
│   ├── .env                   # Example environment file with placeholder data;
│   ├── models/                
│   │   ├── account_model.js   # Defines the account model.
│   │   ├── card_account_model.js # Manages the relationship between cards and accounts.
│   │   ├── card_model.js      # Defines the card model.
│   │   ├── customer_model.js  # Manages customer-related data.
│   │   └── transaction_model.js # Manages transaction-related data.
│   ├── public/                
│   │   ├── stylesheets/
│   │   │   └── style.css      # The stylesheet for the web interface.
│   │   └── index.html         # The main HTML file for the web frontend.
│   ├── routes/                
│   │   ├── account.js         # Routes for account-related operations.
│   │   ├── card.js            # Routes for card-related operations.
│   │   ├── card_account.js    # Routes for linking cards and accounts.
│   │   ├── customer.js        # Routes for customer-related operations.
│   │   ├── index.js           # The entry point for initializing the backend.
│   │   ├── login.js           # Handles login-related routes.
│   │   └── transaction.js     # Routes for transaction-related operations.
│   ├── app.js                 # The main backend application file that sets up and configures middleware.
│   ├── create_token.js        # Handles token creation for secure communication.
│   ├── database.js            # Manages database connections and queries.
│   ├── package-lock.json      # The exact versions of npm dependencies.
│   └── package.json           # Defines the dependencies and scripts for the backend.
├── bank-automat/
│   ├── balance.cpp            # Implements balance-related logic.
│   ├── balance.h              # Header file for balance functionality.
│   ├── balance.ui             # The UI for displaying balance information.
│   ├── bank-automat.pro       # Qt project file for the ATM application.
│   ├── cardmode.cpp           # Implements the logic for card mode.
│   ├── cardmode.h             # Header file for card mode functionality.
│   ├── cardmode.ui            # The UI for card mode.
│   ├── customerdata.cpp       # Implements customer data functionality.
│   ├── customerdata.h         # Header file for customer data functionality.
│   ├── customerdata.ui        # The UI for displaying customer information.
│   ├── environment.cpp        # Implements environment configuration functionality.
│   ├── environment.h          # Header file for environment configuration.
│   ├── login.cpp              # Implements login functionality.
│   ├── login.h                # Header file for login functionality.
│   ├── login.ui               # The UI for the login screen.
│   ├── main.cpp               # The main entry point for the ATM application.
│   ├── mainmenu.cpp           # Implements the main menu logic.
│   ├── mainmenu.h             # Header file for the main menu.
│   ├── mainmenu.ui            # The UI for the main menu.
│   ├── mainwindow.cpp         # Implements the main window logic.
│   ├── mainwindow.h           # Header file for the main window.
│   ├── mainwindow.ui          # The UI for the main window.
│   ├── transaction.cpp        # Implements transaction functionality.
│   ├── transaction.h          # Header file for transaction functionality.
│   ├── transaction.ui         # The UI for displaying transaction information.
│   ├── transfer.cpp           # Implements money transfer logic.
│   ├── transfer.h             # Header file for money transfer functionality.
│   ├── transfer.ui            # The UI for transferring money.
│   ├── withdraw.cpp           # Implements withdrawal functionality.
│   ├── withdraw.h             # Header file for withdrawal functionality.
│   └── withdraw.ui            # The UI for withdrawal operations.
```

# System, Database & Class Architecture

The system is structured into multiple tables:

- **customer:** Stores user details, including first name, last name, profile picture, and account creation date.
- **card:** Holds card-specific data, including debit, credit, and dual card types, PIN codes, status (active, locked, inactive), and expiry date.
- **card_account:** Links cards to accounts and defines whether the link is debit or credit.
- **account:** Tracks account balances, credit limits, and account creation date.
- **transaction:** Logs all ATM transactions, including withdrawals and deposits.

### **Entity-Relationship (ER) Diagram**

Below is the ER diagram representing the database structure:  

<img width="886" alt="ER Diagram" src="https://github.com/andytrix/atm-cash-machine/blob/main/ER-diagram.png" />

### **Deployment Diagram**

Shows how software components are distributed across different hardware and network nodes:  

![Deployment Diagram](https://github.com/andytrix/atm-cash-machine/blob/main/Deployment_Diagram.png)

### **Class Diagram**

Illustrates the relationships between key classes in the project:  

![Class Diagram](https://github.com/andytrix/atm-cash-machine/blob/main/Class_Diagram.png)


