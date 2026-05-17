#ifndef INVENTORY_H
#define INVENTORY_H

#ifdef __cplusplus
extern "C" {
#endif

#define NAME_LEN 40
#define DATA_FILE "inventory.dat"

typedef struct {
    int   id;
    char  name[NAME_LEN];
    int   quantity;
    float price;
    int   is_deleted;   /* 0 = active, 1 = soft-deleted */
} Item;

/* Returns 1 on success, 0 on failure (e.g. duplicate id) */
int add_item(const Item *item);

/* Fills *out; returns 1 if found and active, 0 otherwise */
int get_item(int id, Item *out);

/* Overwrites record in-place; returns 1 on success, 0 if not found */
int update_item(int id, const Item *updated);

/* Soft-deletes the record; returns 1 on success, 0 if not found */
int delete_item(int id);

/* Copies up to max_items active records into buffer.
   Returns the number of items copied. */
int list_items(Item *buffer, int max_items);

#ifdef __cplusplus
}
#endif

#endif /* INVENTORY_H */
