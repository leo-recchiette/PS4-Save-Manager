#include <stdio.h>
#include <sqlite3.h>

#include "saves.h"
#include "common.h"
#include "util.h"


int sqlite3_memvfs_init(const char* vfsName);
int sqlite3_memvfs_dump(sqlite3 *db, const char *zSchema, const char *zFilename);

void* open_sqlite_db(const char* db_path)
{
    uint8_t* db_buf;
    size_t db_size;
    sqlite3 *db;

    if (sqlite3_memvfs_init("orbis_rw") != SQLITE_OK_LOAD_PERMANENTLY)
    {
        LOG("Error loading extension: %s", "memvfs");
        return NULL;
    }

    if (read_buffer(db_path, &db_buf, &db_size) != SUCCESS)
    {
        LOG("Cannot open database file: %s", db_path);
        return NULL;
    }

    // And open that memory with memvfs now that it holds a valid database
    char *memuri = sqlite3_mprintf("file:memdb?ptr=0x%p&sz=%lld&freeonclose=1", db_buf, db_size);
    LOG("Opening '%s' (%ld bytes)...", db_path, db_size);

    if (sqlite3_open_v2(memuri, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_URI, "memvfs") != SQLITE_OK)
    {
        LOG("Error open memvfs: %s", sqlite3_errmsg(db));
        return NULL;
    }
    sqlite3_free(memuri);

    if (sqlite3_exec(db, "PRAGMA journal_mode = OFF;", NULL, NULL, NULL) != SQLITE_OK)
        LOG("Error set pragma: %s", sqlite3_errmsg(db));

    return db;
}

int save_sqlite_db(void* db, const char* db_path)
{
    if (!db)
        return 0;

    LOG("Saving database to %s", db_path);
    if (sqlite3_memvfs_dump(db, NULL, db_path) != SQLITE_OK)
    {
        LOG("Error saving database: %s", sqlite3_errmsg(db));
        return 0;
    }

    return 1;
}
