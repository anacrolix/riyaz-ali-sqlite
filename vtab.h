#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "sqlite3.h"
#include "bridge/bridge.h"

typedef struct {
	int type;
	union {
		int i;
		char *text;
		size_t textLen;
	};
} Value;

typedef struct {
	int errorCode;
	Value value;
} ArrayCursorColumn;

typedef struct {
	int numRows;
	// TODO: Why can't we use columns[] here?
	ArrayCursorColumn *columns;
} ArrayCursorData;

typedef struct {
  int numCols;
  int rowIndex;
  int rowid;
  ArrayCursorData data;
} ArrayCursor;

typedef struct {
	sqlite3_vtab_cursor base;  // base class - must be first
	void *impl;  // pointer to go virtual cursor implementation
	bool isArray; // cursor is fully allocated for C by Filter
	ArrayCursor array;
} go_virtual_cursor;

// C callbacks for array filters
extern int x_next(sqlite3_vtab_cursor *);
extern int x_eof(sqlite3_vtab_cursor*);
extern int x_column(sqlite3_vtab_cursor*, sqlite3_context*, int);
extern int x_rowid(sqlite3_vtab_cursor*, sqlite3_int64*);
extern int x_close(sqlite3_vtab_cursor*);

extern int x_create_tramp(sqlite3*, void*, int, char**, sqlite3_vtab**, char**);
extern int x_connect_tramp(sqlite3*, void*, int, char**, sqlite3_vtab**, char**);
extern int x_best_index_tramp(sqlite3_vtab*, sqlite3_index_info*);
extern int x_disconnect_tramp(sqlite3_vtab*);
extern int x_destroy_tramp(sqlite3_vtab*);
extern int x_open_tramp(sqlite3_vtab*, sqlite3_vtab_cursor**);
extern int x_close_tramp(sqlite3_vtab_cursor*);
extern int x_filter_tramp(sqlite3_vtab_cursor*, int, char*, int, sqlite3_value**);
extern int x_next(sqlite3_vtab_cursor *);
extern int x_next_tramp(go_virtual_cursor *);
extern int x_eof_tramp(sqlite3_vtab_cursor*);
extern int x_column_tramp(sqlite3_vtab_cursor*, sqlite3_context*, int);
extern int x_rowid_tramp(sqlite3_vtab_cursor*, sqlite3_int64*);
extern int x_update_tramp(sqlite3_vtab*, int, sqlite3_value**, sqlite3_int64*);
extern int x_begin_tramp(sqlite3_vtab*);
extern int x_sync_tramp(sqlite3_vtab*);
extern int x_commit_tramp(sqlite3_vtab*);
extern int x_rollback_tramp(sqlite3_vtab*);

typedef void (*overloaded_function)(sqlite3_context*,int,sqlite3_value**);
extern int x_find_function_tramp(sqlite3_vtab*, int, char*, overloaded_function*, void**);
extern void x_overloaded_function_tramp(sqlite3_context*, int, sqlite3_value**);

extern void module_destroy(void*);

static sqlite3_module* _allocate_sqlite3_module() {
  sqlite3_module* module = (sqlite3_module*) _sqlite3_malloc(sizeof(sqlite3_module));
  memset(module, 0, sizeof(sqlite3_module));
  return module;
}

typedef struct go_virtual_table go_virtual_table;
struct go_virtual_table {
  sqlite3_vtab base;  // base class - must be first
  void *impl;  // pointer to go virtual table implementation
};

static int _allocate_virtual_table(sqlite3_vtab **out, void *impl){
  go_virtual_table* table = (go_virtual_table*) _sqlite3_malloc(sizeof(go_virtual_table));
  if (!table) {
    return SQLITE_NOMEM;
  }
  memset(table, 0, sizeof(go_virtual_table));
	 table->impl = impl;
  *out = (sqlite3_vtab*) table;
  return SQLITE_OK;
}

static int _allocate_virtual_cursor(sqlite3_vtab_cursor **out) {
	go_virtual_cursor *cursor = (go_virtual_cursor *) _sqlite3_malloc(sizeof(go_virtual_cursor));
	if (!cursor) {
		return SQLITE_NOMEM;
	}
	memset(cursor, 0, sizeof (go_virtual_cursor));
	*out = (sqlite3_vtab_cursor *) cursor;
	return SQLITE_OK;
}

//int x_eof_tramp(sqlite3_vtab_cursor*);
//int x_column_tramp(sqlite3_vtab_cursor*, sqlite3_context*, int);
//int x_rowid_tramp(sqlite3_vtab_cursor*, sqlite_int64 *pRowid);