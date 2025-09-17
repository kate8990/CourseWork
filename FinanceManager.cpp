#include <iostream>
#include <string>
#include <vector>
#include <ctime>
#include <algorithm>
#include <map>
#include <fstream>
#include <windows.h>
#include <limits> // додано

using namespace std;

class Transaction {
public:
    string category;
    double amount;
    time_t date;
    bool isExpense;

    Transaction(const string& cat, double amt, bool expense)
        : category(cat), amount(amt), isExpense(expense) {
        date = time(nullptr);
    }
};

class Wallet {
private:
    string name;
    double balance;
    vector<Transaction> transactions;

public:
    Wallet(const string& n) : name(n), balance(0) {}

    void deposit(double amt) {
        balance += amt;
        transactions.push_back(Transaction("Поповнення", amt, false));
    }

    bool spend(double amt, const string& category) {
        if (amt > balance) return false;
        balance -= amt;
        transactions.push_back(Transaction(category, amt, true));
        return true;
    }

    double getBalance() const { return balance; }
    string getName() const { return name; }
    const vector<Transaction>& getTransactions() const { return transactions; }
};

class FinanceManager {
private:
    vector<Wallet> wallets;

    time_t periodStart(int periodDays) const {
        time_t now = time(nullptr);
        return now - periodDays * 24 * 60 * 60;
    }

public:
    void addWallet(const string& name) {
        wallets.push_back(Wallet(name));
    }

    Wallet* getWallet(const string& name) {
        for (auto& w : wallets)
            if (w.getName() == name)
                return &w;
        return nullptr;
    }

    void showAllTransactions(int periodDays = 0) const {
        time_t start = (periodDays > 0) ? periodStart(periodDays) : 0;
        for (const auto& w : wallets) { // виправлено
            cout << "\nГаманець: " << w.getName() << " | Баланс: " << w.getBalance() << "\n";
            const auto& t = w.getTransactions();
            bool empty = true;
            for (const auto& tr : t) { // виправлено
                if (periodDays == 0 || tr.date >= start) {
                    tm* timeinfo = localtime(&tr.date);
                    char buffer[20];
                    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M", timeinfo);
                    cout << (tr.isExpense ? "Витрата" : "Поповнення")
                        << " | Категорія: " << tr.category
                        << " | Сума: " << tr.amount
                        << " | Дата: " << buffer << "\n";
                    empty = false;
                }
            }
            if (empty) cout << "Транзакції відсутні за обраний період.\n";
        }
    }

    void saveReport(const string& filename, int periodDays = 0) const {
        ofstream fout(filename);
        if (!fout) { cout << "Помилка відкриття файлу!\n"; return; }
        time_t start = (periodDays > 0) ? periodStart(periodDays) : 0;

        for (const auto& w : wallets) { // виправлено
            fout << "\nГаманець: " << w.getName() << " | Баланс: " << w.getBalance() << "\n";
            const auto& t = w.getTransactions();
            bool empty = true;
            for (const auto& tr : t) { // виправлено
                if (periodDays == 0 || tr.date >= start) {
                    tm* timeinfo = localtime(&tr.date);
                    char buffer[20];
                    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M", timeinfo);
                    fout << (tr.isExpense ? "Витрата" : "Поповнення")
                        << " | Категорія: " << tr.category
                        << " | Сума: " << tr.amount
                        << " | Дата: " << buffer << "\n";
                    empty = false;
                }
            }
            if (empty) fout << "Транзакції відсутні за обраний період.\n";
        }
        fout.close();
        cout << "Звіт збережено у файл " << filename << "\n";
    }

    void topExpenses(int periodDays = 0, int n = 3) const {
        time_t start = (periodDays > 0) ? periodStart(periodDays) : 0;
        vector<Transaction> expenses;
        for (const auto& w : wallets) // виправлено
            for (const auto& tr : w.getTransactions()) // виправлено
                if (tr.isExpense && (periodDays == 0 || tr.date >= start))
                    expenses.push_back(tr);

        sort(expenses.begin(), expenses.end(), [](const Transaction& a, const Transaction& b) { return a.amount > b.amount; });

        cout << "\nТОП-" << n << " витрат:\n";
        for (int i = 0; i < min(n, (int)expenses.size()); i++) {
            tm* timeinfo = localtime(&expenses[i].date);
            char buffer[20];
            strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M", timeinfo);
            cout << i + 1 << ". " << expenses[i].category << " | "
                << expenses[i].amount << " | " << buffer << "\n";
        }
    }

    void topCategories(int periodDays = 0, int n = 3) const {
        time_t start = (periodDays > 0) ? periodStart(periodDays) : 0;
        map<string, double> catSum;
        for (const auto& w : wallets) // виправлено
            for (const auto& tr : w.getTransactions()) // виправлено
                if (tr.isExpense && (periodDays == 0 || tr.date >= start))
                    catSum[tr.category] += tr.amount;

        vector<pair<string, double>> sorted(catSum.begin(), catSum.end());
        sort(sorted.begin(), sorted.end(), [](auto& a, auto& b) { return a.second > b.second; });

        cout << "\nТОП-" << n << " категорій:\n";
        for (int i = 0; i < min(n, (int)sorted.size()); i++)
            cout << i + 1 << ". " << sorted[i].first << " | " << sorted[i].second << "\n";
    }
};

int choosePeriod() {
    int choice;
    cout << "\nОберіть період:\n";
    cout << "1. День\n2. Тиждень\n3. Місяць\n0. Всі часи\n";
    cout << "Ваш вибір: ";
    cin >> choice;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    switch (choice) {
    case 1: return 1;
    case 2: return 7;
    case 3: return 30;
    default: return 0;
    }
}

int main() {
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
    system("color 0A");

    FinanceManager fm;
    int choice;

    do {
        cout << "\n--- МЕНЮ ---\n";
        cout << "1. Додати гаманець/картку\n";
        cout << "2. Поповнити гаманець/картку\n";
        cout << "3. Додати витрату\n";
        cout << "4. Показати всі транзакції\n";
        cout << "5. ТОП-3 витрат\n";
        cout << "6. ТОП-3 категорій\n";
        cout << "7. Зберегти звіт у файл\n";
        cout << "0. Вихід\n";
        cout << "Ваш вибір: ";
        cin >> choice;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        if (choice == 1) {
            string name;
            cout << "Назва гаманця/картки: ";
            getline(cin, name);
            fm.addWallet(name);
            cout << "Гаманець додано!\n";
        }
        else if (choice == 2) {
            string name;
            cout << "Назва гаманця/картки: ";
            getline(cin, name);
            Wallet* w = fm.getWallet(name);
            if (!w) { cout << "Гаманець не знайдено!\n"; continue; }
            double amount;
            cout << "Сума поповнення: ";
            cin >> amount;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            w->deposit(amount);
            cout << "Поповнення успішне!\n";
        }
        else if (choice == 3) {
            string name, category;
            cout << "Назва гаманця/картки: ";
            getline(cin, name);
            Wallet* w = fm.getWallet(name);
            if (!w) { cout << "Гаманець не знайдено!\n"; continue; }
            double amount;
            cout << "Сума витрати: ";
            cin >> amount;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Категорія: ";
            getline(cin, category);
            if (!w->spend(amount, category)) cout << "Недостатньо коштів!\n";
            else cout << "Витрата додана!\n";
        }
        else if (choice == 4) {
            int period = choosePeriod();
            fm.showAllTransactions(period);
        }
        else if (choice == 5) {
            int period = choosePeriod();
            fm.topExpenses(period);
        }
        else if (choice == 6) {
            int period = choosePeriod();
            fm.topCategories(period);
        }
        else if (choice == 7) {
            int period = choosePeriod();
            string filename;
            cout << "Назва файлу: ";
            getline(cin, filename);
            fm.saveReport(filename, period);
        }
        else if (choice != 0) cout << "Невірний вибір!\n";

    } while (choice != 0);

    return 0;
}
