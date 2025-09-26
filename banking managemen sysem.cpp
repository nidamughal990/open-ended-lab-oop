#include <iostream>
#include <string>
#include <vector>
#include <ctime>
#include <iomanip>
#include <limits>
#include <map>

using namespace std;

// Utility function to get current date/time as string
string currentDateTime() {
    time_t now = time(0);
    tm *ltm = localtime(&now);
    char buf[20];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S",ltm);
    return string(buf);
}

enum Currency {
    CUR_PKR,
    CUR_USD,
    CUR_EUR,
    CUR_UNKNOWN
};

string currencyToString(Currency cur) {
    switch(cur) {
        case CUR_PKR: return "PKR";
        case CUR_USD: return "USD";
        case CUR_EUR: return "EUR";
        default: return "Unknown";
    }
}

string currencySymbol(Currency cur) {
    switch(cur) {
        case CUR_PKR: return "Rs.";
        case CUR_USD: return "$";
        case CUR_EUR: return "€";
        default: return "";
    }
}

// Exchange rates relative to PKR (base currency)
map<Currency, double> exchangeRates = {
    {CUR_PKR, 1.0},
    {CUR_USD, 280.0},  // 1 USD = 280 PKR (example rate)
    {CUR_EUR, 310.0}   // 1 EUR = 310 PKR (example rate)
};

// Convert amount from source currency to target currency
double convertCurrency(double amount, Currency fromCur, Currency toCur) {
    if (exchangeRates.find(fromCur) == exchangeRates.end() || exchangeRates.find(toCur) == exchangeRates.end()) {
        return 0.0; // unknown currency
    }
    double amountInPKR = amount * exchangeRates[fromCur];
    return amountInPKR / exchangeRates[toCur];
}

// Account Categories
enum AccountCategory { 
    ACCOUNT_SAVINGS, 
    ACCOUNT_CURRENT, 
    ACCOUNT_FIXED_DEPOSIT 
};

string accountCategoryToString(AccountCategory category) {
    switch(category) {
        case ACCOUNT_SAVINGS: return "Savings";
        case ACCOUNT_CURRENT: return "Current";
        case ACCOUNT_FIXED_DEPOSIT: return "Fixed Deposit";
        default: return "Unknown";
    }
}

// Transaction Categories
enum TransactionCategory { 
    TRANSACTION_DEPOSIT, 
    TRANSACTION_WITHDRAWAL, 
    TRANSACTION_TRANSFER 
};

// Transaction record
struct Transaction {
    string dateTime;
    TransactionCategory category;
    double amountInPKR; // stored in base currency
    Currency currency;  // currency used in transaction
    string details;
};

// Loan record
struct Loan {
    double principalAmountInPKR;
    double annualInterestRate; 
    int termInMonths;
    double monthlyInstallmentInPKR;
    int installmentsPaid;
    bool isActive;

    Loan(double principalInPKR, double interestRate, int termMonths) 
        : principalAmountInPKR(principalInPKR), annualInterestRate(interestRate), termInMonths(termMonths), installmentsPaid(0), isActive(true) {
        double totalInterest = principalAmountInPKR * (annualInterestRate / 100) * (termInMonths / 12.0);
        monthlyInstallmentInPKR = (principalAmountInPKR + totalInterest) / termInMonths;
    }

    void payInstallment() {
        if (installmentsPaid < termInMonths) {
            installmentsPaid++;
            if (installmentsPaid == termInMonths) isActive = false;
        }
    }

    double remainingBalance() const {
        return (termInMonths - installmentsPaid) * monthlyInstallmentInPKR;
    }
};

// Customer Account
class Account {
private:
    static int nextAccountNumber;
    int accountNumber;
    string customerName;
    AccountCategory accountCategory;
    double accountBalanceInPKR; // store balance in base currency PKR
    string accountPassword; // simple password for demo
    vector<Transaction> transactionHistory;
    vector<Loan> loanRecords;

public:
    Account(string name, AccountCategory category, double initialDeposit, Currency depositCurrency, string password) {
        accountNumber = nextAccountNumber++;
        customerName = name;
        accountCategory = category;
        accountBalanceInPKR = convertCurrency(initialDeposit, depositCurrency, CUR_PKR);
        accountPassword = password;
        // Initial deposit transaction
        transactionHistory.push_back({currentDateTime(), TRANSACTION_DEPOSIT, accountBalanceInPKR, depositCurrency, "Initial deposit"});
    }

    int getAccountNumber() const { return accountNumber; }
    string getCustomerName() const { return customerName; }
    AccountCategory getAccountCategory() const { return accountCategory; }
    double getAccountBalanceInPKR() const { return accountBalanceInPKR; }

    bool authenticate(string password) const {
        return password == accountPassword;
    }

    // Deposit amount in specified currency
    void deposit(double amount, Currency currency) {
        if (amount <= 0) {
            cout << "Invalid deposit amount.\n";
            return;
        }
        double amountInPKR = convertCurrency(amount, currency, CUR_PKR);
        accountBalanceInPKR += amountInPKR;
        transactionHistory.push_back({currentDateTime(), TRANSACTION_DEPOSIT, amountInPKR, currency, "Deposit"});
        cout << "Deposit successful. New balance: " << currencySymbol(currency) << fixed << setprecision(2) << convertCurrency(accountBalanceInPKR, CUR_PKR, currency) << " " << currencyToString(currency) << endl;
    }

    // Withdraw amount in specified currency
    bool withdraw(double amount, Currency currency) {
        if (amount <= 0) {
            cout << "Invalid withdrawal amount.\n";
            return false;
        }
        double amountInPKR = convertCurrency(amount, currency, CUR_PKR);
        if (amountInPKR > accountBalanceInPKR) {
            cout << "Insufficient balance.\n";
            return false;
        }
        accountBalanceInPKR -= amountInPKR;
        transactionHistory.push_back({currentDateTime(), TRANSACTION_WITHDRAWAL, amountInPKR, currency, "Withdrawal"});
        cout << "Withdrawal successful. New balance: " << currencySymbol(currency) << fixed << setprecision(2) << convertCurrency(accountBalanceInPKR, CUR_PKR, currency) << " " << currencyToString(currency) << endl;
        return true;
    }

    // Transfer amount in specified currency to another account
    bool transfer(Account &recipientAccount, double amount, Currency currency) {
        if (withdraw(amount, currency)) {
            recipientAccount.deposit(amount, currency);
            transactionHistory.push_back({currentDateTime(), TRANSACTION_TRANSFER, convertCurrency(amount, currency, CUR_PKR), currency, "Transfer to account " + to_string(recipientAccount.getAccountNumber())});
            return true;
        }
        return false;
    }

    void showTransactionHistory() const {
        cout << "Transaction History for Account #" << accountNumber << ":\n";
        for (const auto &transaction : transactionHistory) {
            cout << transaction.dateTime << " | ";
            switch(transaction.category) {
                case TRANSACTION_DEPOSIT: cout << "Deposit"; break;
                case TRANSACTION_WITHDRAWAL: cout << "Withdrawal"; break;
                case TRANSACTION_TRANSFER: cout << "Transfer"; break;
            }
            cout << " | Amount: " << currencySymbol(transaction.currency) << fixed << setprecision(2) << convertCurrency(transaction.amountInPKR, CUR_PKR, transaction.currency) << " " << currencyToString(transaction.currency);
            cout << " | " << transaction.details << endl;
        }
    }

    void applyForLoan(double principal, Currency currency, double interestRate, int termMonths) {
        double principalInPKR = convertCurrency(principal, currency, CUR_PKR);
        loanRecords.push_back(Loan(principalInPKR, interestRate, termMonths));
        // Credit loan principal to account balance (in PKR)
        accountBalanceInPKR += principalInPKR;
        // Record transaction in PKR since balance is credited in PKR
        transactionHistory.push_back({currentDateTime(), TRANSACTION_DEPOSIT, principalInPKR, CUR_PKR, "Loan disbursed"});
        cout << "Loan applied successfully. Loan amount credited to your account.\n";
        cout << "New balance: " << currencySymbol(CUR_PKR) << fixed << setprecision(2) << accountBalanceInPKR << " PKR\n";
    }

    void payLoanInstallment(int loanIndex) {
        if (loanIndex < 0 || loanIndex >= (int)loanRecords.size()) {
            cout << "Invalid loan selection.\n";
            return;
        }
        Loan &loan = loanRecords[loanIndex];
        // Debug info (can be removed later)
        // cout << "Debug: Loan status: " << (loan.isActive ? "Active" : "Paid Off") << ", Installments Paid: " << loan.installmentsPaid << "/" << loan.termInMonths << endl;
        if (!loan.isActive) {
            cout << "Loan already paid off.\n";
            return;
        }
        if (accountBalanceInPKR < loan.monthlyInstallmentInPKR) {
            cout << "Insufficient balance to pay loan installment.\n";
            return;
        }
        accountBalanceInPKR -= loan.monthlyInstallmentInPKR;
        loan.payInstallment();
        transactionHistory.push_back({currentDateTime(), TRANSACTION_WITHDRAWAL, loan.monthlyInstallmentInPKR, CUR_PKR, "Loan payment"});
        cout << "Loan payment successful. Remaining balance: Rs. " << fixed << setprecision(2) << accountBalanceInPKR << " PKR\n";
        if (!loan.isActive) {
            cout << "Loan fully paid off.\n";
        }
    }

    void showLoanDetails() const {
        cout << "Loans for Account #" << accountNumber << ":\n";
        if (loanRecords.empty()) {
            cout << "No loans found.\n";
            return;
        }
        for (size_t i = 0; i < loanRecords.size(); i++) {
            const Loan &loan = loanRecords[i];
            cout << i+1 << ". Principal: Rs. " << fixed << setprecision(2) << loan.principalAmountInPKR
                 << ", Interest Rate: " << loan.annualInterestRate << "%, Term: " << loan.termInMonths << " months"
                 << ", Monthly Installment: Rs. " << loan.monthlyInstallmentInPKR
                 << ", Installments Paid: " << loan.installmentsPaid
                 << ", Status: " << (loan.isActive ? "Active" : "Paid Off") << endl;
        }
    }

    // Show balance in chosen currency
    void showBalance(Currency currency) const {
        double balanceInCurrency = convertCurrency(accountBalanceInPKR, CUR_PKR, currency);
        cout << "Current balance: " << currencySymbol(currency) << fixed << setprecision(2) << balanceInCurrency << " " << currencyToString(currency) << endl;
    }

    // Public getter for loan count
    int getLoanCount() const {
        return (int)loanRecords.size();
    }
};

int Account::nextAccountNumber = 1001;

// Helper function to safely read an integer with validation
int readInt(const string& prompt, int minVal = numeric_limits<int>::min(), int maxVal = numeric_limits<int>::max()) {
    int value;
    while (true) {
        cout << prompt;
        if (cin >> value && value >= minVal && value <= maxVal) {
            cin.ignore(numeric_limits<streamsize>::max(), '\n'); // discard rest of line
            return value;
        } else {
            cout << "Invalid input. Please enter a valid number";
            if (minVal != numeric_limits<int>::min() || maxVal != numeric_limits<int>::max()) {
                cout << " between " << minVal << " and " << maxVal;
            }
            cout << ".\n";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
    }
}

// Helper function to safely read a double with validation
double readDouble(const string& prompt, double minVal = numeric_limits<double>::lowest(), double maxVal = numeric_limits<double>::max()) {
    double value;
    while (true) {
        cout << prompt;
        if (cin >> value && value >= minVal && value <= maxVal) {
            cin.ignore(numeric_limits<streamsize>::max(), '\n'); // discard rest of line
            return value;
        } else {
            cout << "Invalid input. Please enter a valid number";
            if (minVal != numeric_limits<double>::lowest() || maxVal != numeric_limits<double>::max()) {
                cout << " between " << minVal << " and " << maxVal;
            }
            cout << ".\n";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
    }
}

// Helper to read currency choice from user
Currency readCurrency() {
    cout << "Select currency:\n1. PKR\n2. USD\n3. EUR\nChoice: ";
    int choice = readInt("", 1, 3);
    switch(choice) {
        case 1: return CUR_PKR;
        case 2: return CUR_USD;
        case 3: return CUR_EUR;
        default: return CUR_UNKNOWN;
    }
}

// Bank System
class Bank {
private:
    vector<Account> accounts;

    Account* findAccount(int accountNumber) {
        for (auto &account : accounts) {
            if (account.getAccountNumber() == accountNumber) {
                return &account;
            }
        }
        return nullptr;
    }

public:
    void createAccount() {
        string name, password;
        cout << "Enter customer name: ";
        getline(cin, name);

        int accountCategoryChoice = readInt("Select account category:\n1. Savings\n2. Current\n3. Fixed Deposit\nChoice: ", 1, 3);

        Currency currency = readCurrency();

        double initialDeposit = readDouble("Enter initial deposit amount: ", 0.0);

        cout << "Set account password: ";
        getline(cin, password);

        AccountCategory category = static_cast<AccountCategory>(accountCategoryChoice - 1);

        Account newAccount(name, category, initialDeposit, currency, password);
        int accountNumber = newAccount.getAccountNumber();
        accounts.push_back(newAccount);

        cout << "Account created successfully. Account Number: " << accountNumber << endl;
    }

    Account* login() {
        int accountNumber = readInt("Enter account number: ", 1001);
        Account* account = findAccount(accountNumber);
        if (!account) {
            cout << "Account not found.\n";
            return nullptr;
        }
        string password;
        cout << "Enter password: ";
        getline(cin, password);
        if (!account->authenticate(password)) {
            cout << "Authentication failed.\n";
            return nullptr;
        }
        cout << "Login successful.\n";
        return account;
    }

    void run() {
        while (true) {
            cout << "\n--- Bank Management System ---\n";
            cout << "1. Create Account\n2. Login\n3. Exit\nChoose an option: ";
            int choice = readInt("", 1, 3);

            if (choice == 1) {
                createAccount();
            } else if (choice == 2) {
                Account* account = login();
                if (account) accountMenu(*account);
            } else if (choice == 3) {
                cout << "Thank you for using the system.\n";
                break;
            }
        }
    }

    void accountMenu(Account &account) {
        while (true) {
            cout << "\n--- Account Menu (Account #" << account.getAccountNumber() << ") ---\n";
            cout << "1. View Balance\n2. Deposit\n3. Withdraw\n4. Transfer\n5. View Transactions\n6. Apply for Loan\n7. Pay Loan\n8. View Loans\n9. Logout\nChoose an option: ";
            int choice = readInt("", 1, 9);

            if (choice == 1) {
                Currency cur = readCurrency();
                account.showBalance(cur);
            } else if (choice == 2) {
                Currency cur = readCurrency();
                double amount = readDouble("Enter deposit amount: ", 0.01);
                account.deposit(amount, cur);
            } else if (choice == 3) {
                Currency cur = readCurrency();
                double amount = readDouble("Enter withdrawal amount: ", 0.01);
                account.withdraw(amount, cur);
            } else if (choice == 4) {
                int recipientAccountNumber = readInt("Enter recipient account number: ", 1001);
                Currency cur = readCurrency();
                double amount = readDouble("Enter amount to transfer: ", 0.01);
                Account* recipientAccount = findAccount(recipientAccountNumber);
                if (!recipientAccount) {
                    cout << "Recipient account not found.\n";
                } else {
                    if (account.transfer(*recipientAccount, amount, cur)) {
                        cout << "Transfer successful.\n";
                    } else {
                        cout << "Transfer failed.\n";
                    }
                }
            } else if (choice == 5) {
                account.showTransactionHistory();
            } else if (choice == 6) {
                Currency cur = readCurrency();
                double principal = readDouble("Enter loan principal amount: ", 0.01);
                double interestRate = readDouble("Enter annual interest rate (in %): ", 0.0);
                int termMonths = readInt("Enter loan term (months): ", 1);
                account.applyForLoan(principal, cur, interestRate, termMonths);
            } else if (choice == 7) {
                account.showLoanDetails();
                int loanCount = account.getLoanCount();
                if (loanCount == 0) {
                    cout << "No loans to pay.\n";
                    continue;
                }
                int loanNumber = readInt("Enter loan number to pay installment: ", 1, loanCount);
                account.payLoanInstallment(loanNumber - 1);
            } else if (choice == 8) {
                account.showLoanDetails();
            } else if (choice == 9) {
                cout << "Logging out...\n";
                break;
            }
        }
    }
};

int main() {
    Bank bank;
    bank.run();
    return 0;
}
