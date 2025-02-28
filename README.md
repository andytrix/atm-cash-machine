# ATM Cash Machine

Experience a full-featured ATM simulator powered by Qt, Node.js, and MySQL. With secure authentication, dynamic transactions, and comprehensive card management, it brings the authentic feel of real-world banking right to your screen.

<p align="left">
  <img src="https://github.com/user-attachments/assets/049496ca-0f47-4c48-813c-28f51802c0b7" 
       alt="ATM 1" width="480" style="margin-right: 10px;" />
  <img src="https://github.com/user-attachments/assets/2e31eb09-a4d5-45df-8d2d-3d76bdf1618b"
       alt="ATM 2" width="480" />
</p>

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

**3. Configure Environment Variables**

Create a .env file in the backend directory with the following content:  

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
**5. Run the Qt Application**

- Open Qt Creator.
- Load the project file (.pro).
- Build and run the application.

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

# Database Schema

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


