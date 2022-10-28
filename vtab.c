#include <sqlite3ext.h>
#include <stdio.h>
#include "vtab.h"

int x_next(sqlite3_vtab_cursor *base_cursor) {
//	printf("hello from x_next\n");
	go_virtual_cursor *cursor = (void *)base_cursor;
	if (cursor->isArray) {
    cursor->array.rowIndex++;
    cursor->array.rowid++;
    return SQLITE_OK;
	}
	return x_next_tramp(cursor);
}

int x_eof(sqlite3_vtab_cursor *base_cursor) {
  go_virtual_cursor *cursor = (void *)base_cursor;
  if (cursor->isArray) {
    return cursor->array.rowIndex >= cursor->array.data.numRows;
  }
  return x_eof_tramp(base_cursor);
}

int x_column(sqlite3_vtab_cursor *base_cursor, sqlite3_context *context, int column) {
  go_virtual_cursor *cursor = (void *)base_cursor;
  if (!cursor->isArray) {
    return x_column_tramp(base_cursor, context, column);
  }
  printf("column %d\n", column);
  ArrayCursorColumn columnValue = cursor->array.data.columns[cursor->array.rowIndex*cursor->array.numCols+column];
  switch (columnValue.value.type) {
  case SQLITE_INTEGER:
    sqlite3_result_int(context, columnValue.value.i);
  case SQLITE_TEXT:
    sqlite3_result_text(context, columnValue.value.text, columnValue.value.textLen, NULL);
  default:
    sqlite3_result_text(context, "unhandled column value type", -1, SQLITE_STATIC);
    return SQLITE_ERROR;
  }
  return SQLITE_OK;
}

int x_rowid(sqlite3_vtab_cursor *base_cursor, sqlite_int64 *pRowid) {
  go_virtual_cursor *cursor = (void *)base_cursor;
  if (!cursor->isArray) {
    return x_rowid_tramp(base_cursor, pRowid);
  }
  *pRowid = cursor->array.rowid;
  return SQLITE_OK;
}

int x_close(sqlite3_vtab_cursor *base_cursor) {
  // TODO: Free array data
  return x_close_tramp(base_cursor);
}