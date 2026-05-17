#include "inventory.h"
#include <stdio.h>
#include <string.h>

/* ------------------------------------------------------------------ */
/* Internal helpers                                                     */
/* ------------------------------------------------------------------ */

/* Returns the total number of records (including deleted) in the file. */
static long record_count(FILE *fp) {
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    return size / (long)sizeof(Item);
}

/* Seeks fp to the byte offset of record index i (0-based). */
static void seek_to(FILE *fp, long i) {
    fseek(fp, i * (long)sizeof(Item), SEEK_SET);
}

/* Finds the file index of the first record with the given id.
   Returns the index (>= 0) on success, or -1 if not found.
   The record (including deleted ones) is written into *out when found. */
static long find_record(FILE *fp, int id, Item *out) {
    long n = record_count(fp);
    Item tmp;
    for (long i = 0; i < n; i++) {
        seek_to(fp, i);
        if (fread(&tmp, sizeof(Item), 1, fp) != 1) continue;
        if (tmp.id == id) {
            if (out) *out = tmp;
            return i;
        }
    }
    return -1;
}

/* ------------------------------------------------------------------ */
/* Public API                                                           */
/* ------------------------------------------------------------------ */

int add_item(const Item *item) {
    /* Reject non-positive IDs */
    if (item->id <= 0) return 0;

    /* Open for reading to check for duplicate; create if absent. */
    FILE *fp = fopen(DATA_FILE, "rb+");
    if (fp) {
        long idx = find_record(fp, item->id, NULL);
        if (idx >= 0) {          /* duplicate id */
            fclose(fp);
            return 0;
        }
        fclose(fp);
    }

    /* Append the new record. */
    fp = fopen(DATA_FILE, "ab");
    if (!fp) return 0;
    int ok = (fwrite(item, sizeof(Item), 1, fp) == 1);
    fclose(fp);
    return ok;
}

int get_item(int id, Item *out) {
    FILE *fp = fopen(DATA_FILE, "rb");
    if (!fp) return 0;
    Item tmp;
    long idx = find_record(fp, id, &tmp);
    fclose(fp);
    if (idx < 0 || tmp.is_deleted) return 0;
    if (out) *out = tmp;
    return 1;
}

int update_item(int id, const Item *updated) {
    FILE *fp = fopen(DATA_FILE, "rb+");
    if (!fp) return 0;
    Item tmp;
    long idx = find_record(fp, id, &tmp);
    if (idx < 0 || tmp.is_deleted) {
        fclose(fp);
        return 0;
    }
    seek_to(fp, idx);
    int ok = (fwrite(updated, sizeof(Item), 1, fp) == 1);
    fclose(fp);
    return ok;
}

int delete_item(int id) {
    FILE *fp = fopen(DATA_FILE, "rb+");
    if (!fp) return 0;
    Item tmp;
    long idx = find_record(fp, id, &tmp);
    if (idx < 0 || tmp.is_deleted) {
        fclose(fp);
        return 0;
    }
    tmp.is_deleted = 1;
    seek_to(fp, idx);
    int ok = (fwrite(&tmp, sizeof(Item), 1, fp) == 1);
    fclose(fp);
    return ok;
}

int list_items(Item *buffer, int max_items) {
    FILE *fp = fopen(DATA_FILE, "rb");
    if (!fp) return 0;
    long n = record_count(fp);
    int count = 0;
    Item tmp;
    fseek(fp, 0, SEEK_SET);
    for (long i = 0; i < n && count < max_items; i++) {
        if (fread(&tmp, sizeof(Item), 1, fp) != 1) break;
        if (!tmp.is_deleted) {
            buffer[count++] = tmp;
        }
    }
    fclose(fp);
    return count;
}
