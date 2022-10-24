//go:build static
// +build static

package sqlite

// #cgo CFLAGS: -DSQLITE_CORE
// #cgo LDFLAGS: -lsqlite3
import "C"
