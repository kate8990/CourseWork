#include <iostream>
#include <string>
#include <vector>
#include <ctime>
#include <algorithm>
#include <map>
#include <fstream>
#include <limits>
#include <iomanip>

using namespace std;

enum class WalletType { DEBIT, CREDIT };

struct Transaction {
    string category;
    double amount;
    time_t date;
    bool isExpense;
    string walletName;

    Transaction() = default;
    Transaction(const string& wname, const string& cat, double amt, bool expense)
        : category(cat), amount(amt), date(time(nullptr)), isExpense(expense), walletName(wname) {
    }
};

class Wallet {
private:
    string name;
    WalletType type;
    double balance;
    double creditLimit;
    vector<Transaction> transactions;

public:
    Wallet(const string& n, WalletType t, double creditLim = 0.0)
        : name(n), type(t), balance(0.0), creditLimit(creditLim) {
    }

    const string& getName() const { return name; }
    WalletType getType() const { return type; }
    double getBalance() const { return balance; }
    double getCreditLimit() const { return creditLimit; }
    vector<Transaction>& getTransactions() { return transactions; }
    const vector<Transaction>& getTransactionsConst() const { return transactions; }

    void deposit(double amt) {
        if (amt <= 0) return;
        balance += amt;
        transactions.emplace_back(name, "Deposit", amt, false);
    }

    bool spend(double amt, const string& category) {
        if (amt <= 0) return false;
        if (type == WalletType::DEBIT) {
            if (amt > balance) return false;
            balance -= amt;
            transactions.emplace_back(name, category, amt, true);
            return true;
        }
        else { // CREDIT
            if (balance - amt < -creditLimit) return false;
            balance -= amt;
            transactions.emplace_back(name, category, amt, true);
            return true;
        }
    }
};

class FinanceManager {
private:
    vector<Wallet> wallets;

    static time_t nowTime() {
        return time(nullptr);
    }

    static time_t periodStartDays(int days) {
        if (days <= 0) return 0;
        time_t now = nowTime();
        return now - static_cast<time_t>(days) * 24 * 60 * 60;
    }

    static string formatTime(time_t t) {
        if (t == 0) return string("-");
        tm local_tm;
#if defined(_MSC_VER)
        localtime_s(&local_tm, &t);
#else
        localtime_r(&t, &local_tm);
#endif
        char buf[64];
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M", &local_tm);
        return string(buf);
    }

public:
    bool addWallet(const string& name, WalletType type, double creditLimit = 0.0) {
        if (getWallet(name) != nullptr) return false;
        wallets.emplace_back(name, type, creditLimit);
        return true;
    }

    Wallet* getWallet(const string& name) {
        for (auto& w : wallets)
            if (w.getName() == name) return &w;
        return nullptr;
    }

    const vector<Wallet>& getAllWallets() const { return wallets; }

    void depositToWallet(const string& name, double amt) {
        Wallet* w = getWallet(name);
        if (!w) { cout << "Wallet not found.\n"; return; }
        w->deposit(amt);
        cout << "Deposit successful. Balance: " << fixed << setprecision(2) << w->getBalance() << "\n";
    }

    void spendFromWallet(const string& name, double amt, const string& category) {
        Wallet* w = getWallet(name);
        if (!w) { cout << "Wallet not found.\n"; return; }
        if (w->spend(amt, category)) {
            cout << "Expense added. Balance: " << fixed << setprecision(2) << w->getBalance() << "\n";
        }
        else {
            cout << "Insufficient funds or credit limit.\n";
        }
    }

    void showAllTransactions(int days = 0) const {
        time_t start = (days > 0) ? periodStartDays(days) : 0;
        bool any = false;
        for (const auto& w : wallets) {
            cout << "\nWallet: " << w.getName()
                << " | Type: " << (w.getType() == WalletType::DEBIT ? "DEBIT" : "CREDIT")
                << " | Balance: " << fixed << setprecision(2) << w.getBalance() << "\n";
            const auto& tr = w.getTransactionsConst();
            bool empty = true;
            for (const auto& t : tr) {
                if (start == 0 || t.date >= start) {
                    cout << (t.isExpense ? "Expense" : "Deposit")
                        << " | Wallet: " << t.walletName
                        << " | Category: " << t.category
                        << " | Amount: " << fixed << setprecision(2) << t.amount
                        << " | Date: " << formatTime(t.date) << "\n";
                    empty = false;
                    any = true;
                }
            }
            if (empty) cout << "  No transactions for the selected period.\n";
        }
        if (!any) cout << "\nNo transactions found for the selected period.\n";
    }

    void saveReportToFile(const string& filename, int days = 0) const {
        ofstream fout(filename);
        if (!fout) {
            cout << "Failed to open file for writing\n";
            return;
        }
        time_t start = (days > 0) ? periodStartDays(days) : 0;
        fout << "REPORT (period days: " << days << ")\n";
        for (const auto& w : wallets) {
            fout << "\nWallet: " << w.getName() << " | Type: "
                << (w.getType() == WalletType::DEBIT ? "DEBIT" : "CREDIT")
                << " | Balance: " << fixed << setprecision(2) << w.getBalance() << "\n";
            const auto& tr = w.getTransactionsConst();
            bool empty = true;
            for (const auto& t : tr) {
                if (start == 0 || t.date >= start) {
                    fout << (t.isExpense ? "Expense" : "Deposit")
                        << " | Wallet: " << t.walletName
                        << " | Category: " << t.category
                        << " | Amount: " << fixed << setprecision(2) << t.amount
                        << " | Date: " << formatTime(t.date) << "\n";
                    empty = false;
                }
            }
            if (empty) fout << "  No transactions for the selected period.\n";
        }
        fout.close();
        cout << "Report saved to file: " << filename << "\n";
    }

    vector<Transaction> collectTransactions(int days = 0) const {
        time_t start = (days > 0) ? periodStartDays(days) : 0;
        vector<Transaction> all;
        for (const auto& w : wallets) {
            for (const auto& t : w.getTransactionsConst()) {
                if (start == 0 || t.date >= start) all.push_back(t);
            }
        }
        return all;
    }

    vector<Transaction> topExpenses(int days = 0, int topN = 3) const {
        auto all = collectTransactions(days);
        vector<Transaction> expenses;
        for (const auto& t : all) if (t.isExpense) expenses.push_back(t);
        sort(expenses.begin(), expenses.end(), [](const Transaction& a, const Transaction& b) {
            return a.amount > b.amount;
            });
        if ((int)expenses.size() > topN) expenses.resize(topN);
        return expenses;
    }

    vector<pair<string, double>> topCategories(int days = 0, int topN = 3) const {
        auto all = collectTransactions(days);
        map<string, double> sums;
        for (const auto& t : all) if (t.isExpense) sums[t.category] += t.amount;
        vector<pair<string, double>> vec(sums.begin(), sums.end());
        sort(vec.begin(), vec.end(), [](const pair<string, double>& a, const pair<string, double>& b) {
            return a.second > b.second;
            });
        if ((int)vec.size() > topN) vec.resize(topN);
        return vec;
    }

    void saveTopExpensesToFile(const string& filename, int days = 0, int topN = 3) const {
        ofstream fout(filename);
        if (!fout) { cout << "Failed to open file for writing\n"; return; }
        fout << "TOP-" << topN << " EXPENSES (last " << days << " days)\n";
        auto top = topExpenses(days, topN);
        if (top.empty()) fout << "No expenses for the period.\n";
        for (size_t i = 0; i < top.size(); ++i) {
            fout << i + 1 << ". Wallet: " << top[i].walletName
                << " | Category: " << top[i].category
                << " | Amount: " << fixed << setprecision(2) << top[i].amount
                << " | Date: " << formatTime(top[i].date) << "\n";
        }
        fout.close();
        cout << "Top expenses saved to file: " << filename << "\n";
    }

    void saveTopCategoriesToFile(const string& filename, int days = 0, int topN = 3) const {
        ofstream fout(filename);
        if (!fout) { cout << "Failed to open file for writing\n"; return; }
        fout << "TOP-" << topN << " CATEGORIES (last " << days << " days)\n";
        auto top = topCategories(days, topN);
        if (top.empty()) fout << "No expenses for the period.\n";
        for (size_t i = 0; i < top.size(); ++i) {
            fout << i + 1 << ". Category: " << top[i].first
                << " | Sum: " << fixed << setprecision(2) << top[i].second << "\n";
        }
        fout.close();
        cout << "Top categories saved to file: " << filename << "\n";
    }

    void printTopExpensesConsole(int days = 0, int topN = 3) const {
        auto top = topExpenses(days, topN);
        cout << "\n== TOP-" << topN << " expenses (last " << (days == 0 ? "all time" : to_string(days) + " days") << ") ==\n";
        if (top.empty()) { cout << "No expenses for the period.\n"; return; }
        for (size_t i = 0; i < top.size(); ++i) {
            cout << i + 1 << ". Wallet: " << top[i].walletName
                << " | Category: " << top[i].category
                << " | Amount: " << fixed << setprecision(2) << top[i].amount
                << " | Date: " << formatTime(top[i].date) << "\n";
        }
    }

    void printTopCategoriesConsole(int days = 0, int topN = 3) const {
        auto top = topCategories(days, topN);
        cout << "\n== TOP-" << topN << " categories (last " << (days == 0 ? "all time" : to_string(days) + " days") << ") ==\n";
        if (top.empty()) { cout << "No expenses for the period.\n"; return; }
        for (size_t i = 0; i < top.size(); ++i) {
            cout << i + 1 << ". Category: " << top[i].first
                << " | Sum: " << fixed << setprecision(2) << top[i].second << "\n";
        }
    }
};

int choosePeriodDays() {
    cout << "\nChoose period:\n1) Day\n2) Week\n3) Month\n0) All time\nYour choice: ";
    int ch;
    if (!(cin >> ch)) { cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n'); return 0; }
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    switch (ch) {
    case 1: return 1;
    case 2: return 7;
    case 3: return 30;
    default: return 0;
    }
}

// Функція для додавання тестових транзакцій з різними датами
void addDemoTransactions(FinanceManager& fm) {
    // Для Cash
    Wallet* w = fm.getWallet("Cash");
    if (w) {
        w->deposit(500); // сьогодні
        Transaction t1("Cash", "Food", 120, true); t1.date = time(nullptr) - 24 * 60 * 60; // 1 день назад
        w->getTransactions().push_back(t1);
        Transaction t2("Cash", "Taxi", 50, true); t2.date = time(nullptr) - 8 * 24 * 60 * 60; // 8 днів назад
        w->getTransactions().push_back(t2);
        Transaction t3("Cash", "Coffee", 15, true); t3.date = time(nullptr) - 30 * 24 * 60 * 60; // 30 днів назад
        w->getTransactions().push_back(t3);
    }
    // Для VISA_Card
    Wallet* v = fm.getWallet("VISA_Card");
    if (v) {
        v->deposit(1000); // сьогодні
        Transaction t1("VISA_Card", "Groceries", 250, true); t1.date = time(nullptr) - 3 * 24 * 60 * 60; // 3 дні назад
        v->getTransactions().push_back(t1);
        Transaction t2("VISA_Card", "Sport", 100, true); t2.date = time(nullptr) - 16 * 24 * 60 * 60; // 16 днів назад
        v->getTransactions().push_back(t2);
    }
    // Для Credit_MC
    Wallet* c = fm.getWallet("Credit_MC");
    if (c) {
        c->deposit(700); // сьогодні
        Transaction t1("Credit_MC", "Electronics", 400, true); t1.date = time(nullptr) - 5 * 24 * 60 * 60; // 5 днів назад
        c->getTransactions().push_back(t1);
        Transaction t2("Credit_MC", "Travel", 300, true); t2.date = time(nullptr) - 31 * 24 * 60 * 60; // 31 день назад
        c->getTransactions().push_back(t2);
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    FinanceManager fm;
    cout << "=== Personal Finance Management System ===\n";

    // sample wallets
    fm.addWallet("Cash", WalletType::DEBIT);
    fm.addWallet("VISA_Card", WalletType::DEBIT);
    fm.addWallet("Credit_MC", WalletType::CREDIT, 1000.0);

    // Додаємо тестові транзакції для демонстрації фільтрації
    addDemoTransactions(fm);

    while (true) {
        cout << "\n--- MENU ---\n";
        cout << "1. Add wallet/card\n";
        cout << "2. Deposit to wallet/card\n";
        cout << "3. Add expense\n";
        cout << "4. Show all transactions (by period)\n";
        cout << "5. Show TOP-3 expenses (week/month)\n";
        cout << "6. Show TOP-3 categories (week/month)\n";
        cout << "7. Save report to file\n";
        cout << "8. Save TOP-3 expenses to file (week/month)\n";
        cout << "9. Save TOP-3 categories to file (week/month)\n";
        cout << "0. Exit\n";
        cout << "Choice: ";

        int choice;
        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input\n";
            continue;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        if (choice == 0) break;

        if (choice == 1) {
            string name;
            cout << "Enter wallet/card name: ";
            getline(cin, name);
            cout << "Type (1=DEBIT, 2=CREDIT): ";
            int t; cin >> t; cin.ignore(numeric_limits<streamsize>::max(), '\n');
            if (t == 1) {
                if (fm.addWallet(name, WalletType::DEBIT)) cout << "DEBIT wallet added\n";
                else cout << "Wallet with this name already exists\n";
            }
            else {
                double lim; cout << "Enter credit limit (e.g. 1000): "; cin >> lim; cin.ignore(numeric_limits<streamsize>::max(), '\n');
                if (fm.addWallet(name, WalletType::CREDIT, lim)) cout << "CREDIT card added\n";
                else cout << "Wallet with this name already exists\n";
            }
        }
        else if (choice == 2) {
            string name; double amt;
            cout << "Wallet/card name to deposit into: "; getline(cin, name);
            cout << "Deposit amount: "; cin >> amt; cin.ignore(numeric_limits<streamsize>::max(), '\n');
            fm.depositToWallet(name, amt);
        }
        else if (choice == 3) {
            string name, cat; double amt;
            cout << "Wallet/card name for expense: "; getline(cin, name);
            cout << "Expense amount: "; cin >> amt; cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Expense category (e.g. Groceries, Transport): "; getline(cin, cat);
            fm.spendFromWallet(name, amt, cat);
        }
        else if (choice == 4) {
            int days = choosePeriodDays();
            fm.showAllTransactions(days);
        }
        else if (choice == 5) {
            cout << "1) Week  2) Month  0) All time\nYour choice: ";
            int ch; cin >> ch; cin.ignore(numeric_limits<streamsize>::max(), '\n');
            int days = (ch == 1 ? 7 : (ch == 2 ? 30 : 0));
            fm.printTopExpensesConsole(days, 3);
        }
        else if (choice == 6) {
            cout << "1) Week  2) Month  0) All time\nYour choice: ";
            int ch; cin >> ch; cin.ignore(numeric_limits<streamsize>::max(), '\n');
            int days = (ch == 1 ? 7 : (ch == 2 ? 30 : 0));
            fm.printTopCategoriesConsole(days, 3);
        }
        else if (choice == 7) {
            int days = choosePeriodDays();
            string filename;
            cout << "Filename for report (e.g. report.txt): ";
            getline(cin, filename);
            fm.saveReportToFile(filename, days);
        }
        else if (choice == 8) {
            cout << "1) Week  2) Month  0) All time\nYour choice: ";
            int ch; cin >> ch; cin.ignore(numeric_limits<streamsize>::max(), '\n');
            int days = (ch == 1 ? 7 : (ch == 2 ? 30 : 0));
            string filename;
            cout << "Filename for TOP expenses (e.g. top_exp.txt): ";
            getline(cin, filename);
            fm.saveTopExpensesToFile(filename, days, 3);
        }
        else if (choice == 9) {
            cout << "1) Week  2) Month  0) All time\nYour choice: ";
            int ch; cin >> ch; cin.ignore(numeric_limits<streamsize>::max(), '\n');
            int days = (ch == 1 ? 7 : (ch == 2 ? 30 : 0));
            string filename;
            cout << "Filename for TOP categories (e.g. top_cat.txt): ";
            getline(cin, filename);
            fm.saveTopCategoriesToFile(filename, days, 3);
        }
        else {
            cout << "Unknown command\n";
        }
    }

    cout << "Thank you! Goodbye.\n";
    return 0;
}
