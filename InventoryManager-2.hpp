#ifndef INVENTORY_MANAGER_HPP
#define INVENTORY_MANAGER_HPP

#include "inventory.h"
#include <vector>
#include <algorithm>
#include <string>

class InventoryManager {
public:
    /* ---- CRUD wrappers ---- */

    bool addItem(const Item &item) {
        return add_item(&item) == 1;
    }

    bool getItem(int id, Item &out) {
        return get_item(id, &out) == 1;
    }

    bool updateItem(int id, const Item &updated) {
        return update_item(id, &updated) == 1;
    }

    bool deleteItem(int id) {
        return delete_item(id) == 1;
    }

    /* ---- List with STL sort ---- */

    /* Returns active items sorted by id. */
    std::vector<Item> listSortedById() {
        return fetchAndSort([](const Item &a, const Item &b){
            return a.id < b.id;
        });
    }

    /* Returns active items sorted by name (case-insensitive). */
    std::vector<Item> listSortedByName() {
        return fetchAndSort([](const Item &a, const Item &b){
            return std::string(a.name) < std::string(b.name);
        });
    }

private:
    static constexpr int MAX_ITEMS = 4096;

    template<typename Comparator>
    std::vector<Item> fetchAndSort(Comparator cmp) {
        Item buf[MAX_ITEMS];
        int n = list_items(buf, MAX_ITEMS);
        std::vector<Item> items(buf, buf + n);   /* STL vector */
        std::sort(items.begin(), items.end(), cmp); /* STL sort  */
        return items;
    }
};

#endif /* INVENTORY_MANAGER_HPP */
