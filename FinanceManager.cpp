#include <iostream>
#include <string>
#include <vector>
#include <ctime>
#include <algorithm>
#include <map>
#include <fstream>
#include <windows.h>
#include <limits> // ������

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
        transactions.push_back(Transaction("����������", amt, false));
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
        for (const auto& w : wallets) { // ����������
            cout << "\n��������: " << w.getName() << " | ������: " << w.getBalance() << "\n";
            const auto& t = w.getTransactions();
            bool empty = true;
            for (const auto& tr : t) { // ����������
                if (periodDays == 0 || tr.date >= start) {
                    tm* timeinfo = localtime(&tr.date);
                    char buffer[20];
                    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M", timeinfo);
                    cout << (tr.isExpense ? "�������" : "����������")
                        << " | ��������: " << tr.category
                        << " | ����: " << tr.amount
                        << " | ����: " << buffer << "\n";
                    empty = false;
                }
            }
            if (empty) cout << "���������� ������ �� ������� �����.\n";
        }
    }

    void saveReport(const string& filename, int periodDays = 0) const {
        ofstream fout(filename);
        if (!fout) { cout << "������� �������� �����!\n"; return; }
        time_t start = (periodDays > 0) ? periodStart(periodDays) : 0;

        for (const auto& w : wallets) { // ����������
            fout << "\n��������: " << w.getName() << " | ������: " << w.getBalance() << "\n";
            const auto& t = w.getTransactions();
            bool empty = true;
            for (const auto& tr : t) { // ����������
                if (periodDays == 0 || tr.date >= start) {
                    tm* timeinfo = localtime(&tr.date);
                    char buffer[20];
                    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M", timeinfo);
                    fout << (tr.isExpense ? "�������" : "����������")
                        << " | ��������: " << tr.category
                        << " | ����: " << tr.amount
                        << " | ����: " << buffer << "\n";
                    empty = false;
                }
            }
            if (empty) fout << "���������� ������ �� ������� �����.\n";
        }
        fout.close();
        cout << "��� ��������� � ���� " << filename << "\n";
    }

    void topExpenses(int periodDays = 0, int n = 3) const {
        time_t start = (periodDays > 0) ? periodStart(periodDays) : 0;
        vector<Transaction> expenses;
        for (const auto& w : wallets) // ����������
            for (const auto& tr : w.getTransactions()) // ����������
                if (tr.isExpense && (periodDays == 0 || tr.date >= start))
                    expenses.push_back(tr);

        sort(expenses.begin(), expenses.end(), [](const Transaction& a, const Transaction& b) { return a.amount > b.amount; });

        cout << "\n���-" << n << " ������:\n";
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
        for (const auto& w : wallets) // ����������
            for (const auto& tr : w.getTransactions()) // ����������
                if (tr.isExpense && (periodDays == 0 || tr.date >= start))
                    catSum[tr.category] += tr.amount;

        vector<pair<string, double>> sorted(catSum.begin(), catSum.end());
        sort(sorted.begin(), sorted.end(), [](auto& a, auto& b) { return a.second > b.second; });

        cout << "\n���-" << n << " ��������:\n";
        for (int i = 0; i < min(n, (int)sorted.size()); i++)
            cout << i + 1 << ". " << sorted[i].first << " | " << sorted[i].second << "\n";
    }
};

int choosePeriod() {
    int choice;
    cout << "\n������ �����:\n";
    cout << "1. ����\n2. �������\n3. ̳����\n0. �� ����\n";
    cout << "��� ����: ";
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
        cout << "\n--- ���� ---\n";
        cout << "1. ������ ��������/������\n";
        cout << "2. ��������� ��������/������\n";
        cout << "3. ������ �������\n";
        cout << "4. �������� �� ����������\n";
        cout << "5. ���-3 ������\n";
        cout << "6. ���-3 ��������\n";
        cout << "7. �������� ��� � ����\n";
        cout << "0. �����\n";
        cout << "��� ����: ";
        cin >> choice;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        if (choice == 1) {
            string name;
            cout << "����� �������/������: ";
            getline(cin, name);
            fm.addWallet(name);
            cout << "�������� ������!\n";
        }
        else if (choice == 2) {
            string name;
            cout << "����� �������/������: ";
            getline(cin, name);
            Wallet* w = fm.getWallet(name);
            if (!w) { cout << "�������� �� ��������!\n"; continue; }
            double amount;
            cout << "���� ����������: ";
            cin >> amount;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            w->deposit(amount);
            cout << "���������� ������!\n";
        }
        else if (choice == 3) {
            string name, category;
            cout << "����� �������/������: ";
            getline(cin, name);
            Wallet* w = fm.getWallet(name);
            if (!w) { cout << "�������� �� ��������!\n"; continue; }
            double amount;
            cout << "���� �������: ";
            cin >> amount;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "��������: ";
            getline(cin, category);
            if (!w->spend(amount, category)) cout << "����������� �����!\n";
            else cout << "������� ������!\n";
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
            cout << "����� �����: ";
            getline(cin, filename);
            fm.saveReport(filename, period);
        }
        else if (choice != 0) cout << "������� ����!\n";

    } while (choice != 0);

    return 0;
}
