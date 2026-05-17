#include "InventoryManager.hpp"
#include <iostream>
#include <iomanip>
#include <limits>
#include <string>
#include <cstring>
#include <vector>

/* ------------------------------------------------------------------ */
/* Colour / style helpers (ANSI – falls back cleanly on plain terminals)*/
/* ------------------------------------------------------------------ */
namespace Clr {
    const char *RESET  = "\033[0m";
    const char *BOLD   = "\033[1m";
    const char *RED    = "\033[31m";
    const char *GREEN  = "\033[32m";
    const char *YELLOW = "\033[33m";
    const char *CYAN   = "\033[36m";
    const char *WHITE  = "\033[97m";
}

/* ------------------------------------------------------------------ */
/* Input helpers                                                        */
/* ------------------------------------------------------------------ */

static void clearInput() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

static int readPositiveInt(const std::string &prompt) {
    int v;
    while (true) {
        std::cout << prompt;
        if (std::cin >> v && v > 0) { clearInput(); return v; }
        std::cout << Clr::RED << "  ✖  Must be a positive integer. Try again.\n" << Clr::RESET;
        clearInput();
    }
}

static int readNonNegativeInt(const std::string &prompt) {
    int v;
    while (true) {
        std::cout << prompt;
        if (std::cin >> v && v >= 0) { clearInput(); return v; }
        std::cout << Clr::RED << "  ✖  Must be 0 or greater. Try again.\n" << Clr::RESET;
        clearInput();
    }
}

static float readNonNegativeFloat(const std::string &prompt) {
    float v;
    while (true) {
        std::cout << prompt;
        if (std::cin >> v && v >= 0.0f) { clearInput(); return v; }
        std::cout << Clr::RED << "  ✖  Must be 0.00 or greater. Try again.\n" << Clr::RESET;
        clearInput();
    }
}

static std::string readNonEmptyLine(const std::string &prompt) {
    std::string s;
    while (true) {
        std::cout << prompt;
        std::getline(std::cin, s);
        if (!s.empty()) return s;
        std::cout << Clr::RED << "  ✖  Name must not be empty. Try again.\n" << Clr::RESET;
    }
}

/* ------------------------------------------------------------------ */
/* Display helpers                                                      */
/* ------------------------------------------------------------------ */

static void printHeader() {
    std::cout << Clr::BOLD << Clr::CYAN
              << "\n╔══════════════════════════════════════════════════════╗\n"
              << "║          HYBRID INVENTORY MANAGER  v1.0              ║\n"
              << "╚══════════════════════════════════════════════════════╝\n"
              << Clr::RESET;
}

static void printMenu() {
    std::cout << Clr::BOLD << Clr::WHITE
              << "\n  ┌─────────────────────────┐\n"
              << "  │        MAIN MENU        │\n"
              << "  ├─────────────────────────┤\n"
              << "  │  1 │ Add item            │\n"
              << "  │  2 │ View item           │\n"
              << "  │  3 │ Update item         │\n"
              << "  │  4 │ Delete item         │\n"
              << "  │  5 │ List all items      │\n"
              << "  │  6 │ Exit                │\n"
              << "  └─────────────────────────┘\n"
              << Clr::RESET;
    std::cout << Clr::YELLOW << "  Choice: " << Clr::RESET;
}

static void printItem(const Item &it) {
    std::cout << Clr::CYAN
              << "  ┌─────────────────────────────────────┐\n"
              << "  │ ID       : " << Clr::WHITE << std::setw(26) << std::left << it.id     << Clr::CYAN << "│\n"
              << "  │ Name     : " << Clr::WHITE << std::setw(26) << std::left << it.name   << Clr::CYAN << "│\n"
              << "  │ Quantity : " << Clr::WHITE << std::setw(26) << std::left << it.quantity << Clr::CYAN << "│\n"
              << "  │ Price    : $" << Clr::WHITE << std::setw(25) << std::left
              << std::fixed << std::setprecision(2) << it.price   << Clr::CYAN << "│\n"
              << "  └─────────────────────────────────────┘\n"
              << Clr::RESET;
}

static void printTable(const std::vector<Item> &items) {
    if (items.empty()) {
        std::cout << Clr::YELLOW << "  (no active items)\n" << Clr::RESET;
        return;
    }
    std::cout << Clr::BOLD << Clr::CYAN
              << "  ┌──────┬──────────────────────────────────────┬──────────┬───────────┐\n"
              << "  │  ID  │ Name                                 │  Qty     │   Price   │\n"
              << "  ├──────┼──────────────────────────────────────┼──────────┼───────────┤\n"
              << Clr::RESET;
    for (const auto &it : items) {
        std::cout << Clr::WHITE
                  << "  │ " << std::setw(4)  << std::left << it.id      << " │ "
                  << std::setw(36) << std::left  << it.name     << " │ "
                  << std::setw(8)  << std::right << it.quantity << " │ "
                  << "$" << std::setw(8) << std::right << std::fixed << std::setprecision(2) << it.price
                  << " │\n" << Clr::RESET;
    }
    std::cout << Clr::CYAN
              << "  └──────┴──────────────────────────────────────┴──────────┴───────────┘\n"
              << Clr::RESET;
    std::cout << Clr::GREEN << "  Total active items: " << items.size() << "\n" << Clr::RESET;
}

/* ------------------------------------------------------------------ */
/* Build an Item from user input                                        */
/* ------------------------------------------------------------------ */

static Item buildItemFromInput(int id = -1) {
    Item it{};
    it.id = (id > 0) ? id : readPositiveInt("  ID       : ");

    std::string name = readNonEmptyLine("  Name     : ");
    std::strncpy(it.name, name.c_str(), NAME_LEN - 1);
    it.name[NAME_LEN - 1] = '\0';

    it.quantity   = readNonNegativeInt  ("  Quantity : ");
    it.price      = readNonNegativeFloat("  Price $  : ");
    it.is_deleted = 0;
    return it;
}

/* ------------------------------------------------------------------ */
/* Menu actions                                                         */
/* ------------------------------------------------------------------ */

static void doAdd(InventoryManager &mgr) {
    std::cout << Clr::BOLD << "\n── Add New Item ──────────────────────\n" << Clr::RESET;
    Item it = buildItemFromInput();
    if (mgr.addItem(it)) {
        std::cout << Clr::GREEN << "  ✔  Item added successfully.\n" << Clr::RESET;
    } else {
        std::cout << Clr::RED << "  ✖  Failed to add item (duplicate ID or invalid data).\n" << Clr::RESET;
    }
}

static void doView(InventoryManager &mgr) {
    std::cout << Clr::BOLD << "\n── View Item ─────────────────────────\n" << Clr::RESET;
    int id = readPositiveInt("  Enter ID : ");
    Item it{};
    if (mgr.getItem(id, it)) {
        printItem(it);
    } else {
        std::cout << Clr::RED << "  ✖  Item not found or has been deleted.\n" << Clr::RESET;
    }
}

static void doUpdate(InventoryManager &mgr) {
    std::cout << Clr::BOLD << "\n── Update Item ───────────────────────\n" << Clr::RESET;
    int id = readPositiveInt("  Enter ID to update : ");
    Item existing{};
    if (!mgr.getItem(id, existing)) {
        std::cout << Clr::RED << "  ✖  Item not found or has been deleted.\n" << Clr::RESET;
        return;
    }
    std::cout << Clr::YELLOW << "  Current record:\n" << Clr::RESET;
    printItem(existing);
    std::cout << Clr::YELLOW << "  Enter new values (leave name same if unchanged):\n" << Clr::RESET;
    Item updated = buildItemFromInput(id);   /* keep same id */
    if (mgr.updateItem(id, updated)) {
        std::cout << Clr::GREEN << "  ✔  Item updated.\n" << Clr::RESET;
    } else {
        std::cout << Clr::RED << "  ✖  Update failed.\n" << Clr::RESET;
    }
}

static void doDelete(InventoryManager &mgr) {
    std::cout << Clr::BOLD << "\n── Delete Item ───────────────────────\n" << Clr::RESET;
    int id = readPositiveInt("  Enter ID to delete : ");
    if (mgr.deleteItem(id)) {
        std::cout << Clr::GREEN << "  ✔  Item soft-deleted (will not appear in future views).\n" << Clr::RESET;
    } else {
        std::cout << Clr::RED << "  ✖  Item not found or already deleted.\n" << Clr::RESET;
    }
}

static void doList(InventoryManager &mgr) {
    std::cout << Clr::BOLD << "\n── List All Active Items (sorted by ID) ──\n" << Clr::RESET;
    auto items = mgr.listSortedById();
    printTable(items);
}

/* ------------------------------------------------------------------ */
/* Entry point                                                          */
/* ------------------------------------------------------------------ */

int main() {
    InventoryManager mgr;
    printHeader();

    int choice = 0;
    while (true) {
        printMenu();

        if (!(std::cin >> choice)) {
            clearInput();
            std::cout << Clr::RED << "  ✖  Invalid input.\n" << Clr::RESET;
            continue;
        }
        clearInput();

        switch (choice) {
            case 1: doAdd(mgr);    break;
            case 2: doView(mgr);   break;
            case 3: doUpdate(mgr); break;
            case 4: doDelete(mgr); break;
            case 5: doList(mgr);   break;
            case 6:
                std::cout << Clr::GREEN << "\n  Goodbye! Data saved to " << DATA_FILE << "\n\n" << Clr::RESET;
                return 0;
            default:
                std::cout << Clr::RED << "  ✖  Please enter a number between 1 and 6.\n" << Clr::RESET;
        }
    }
}
