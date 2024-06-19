/* (c) 2024 initia labs. Licensed under BUSL-1.1 */

#ifndef __LIBMOVEVM__
#define __LIBMOVEVM__

/* Generated with cbindgen:0.26.0 */

/* Warning, this file is autogenerated by cbindgen. Don't modify this manually. */

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>


enum ErrnoValue {
  ErrnoValue_Success = 0,
  ErrnoValue_Other = 1,
};
typedef int32_t ErrnoValue;

/**
 * This enum gives names to the status codes returned from Go callbacks to Rust.
 * The Go code will return one of these variants when returning.
 *
 * 0 means no error, all the other cases are some sort of error.
 *
 */
enum GoError {
  GoError_None = 0,
  /**
   * Go panicked for an unexpected reason.
   */
  GoError_Panic = 1,
  /**
   * Go received a bad argument from Rust
   */
  GoError_BadArgument = 2,
  /**
   * Error while trying to serialize data in Go code (typically json.Marshal)
   */
  GoError_CannotSerialize = 3,
  /**
   * An error happened during normal operation of a Go callback, which should be fed back to the contract
   */
  GoError_User = 4,
  /**
   * Unimplemented
   */
  GoError_Unimplemented = 5,
  /**
   * An error type that should never be created by us. It only serves as a fallback for the i32 to GoError conversion.
   */
  GoError_Other = -1,
};
typedef int32_t GoError;

typedef struct {

} vm_t;

/**
 * An optional Vector type that requires explicit creation and destruction
 * and can be sent via FFI.
 * It can be created from `Option<Vec<u8>>` and be converted into `Option<Vec<u8>>`.
 *
 * This type is always created in Rust and always dropped in Rust.
 * If Go code want to create it, it must instruct Rust to do so via the
 * [`new_unmanaged_vector`] FFI export. If Go code wants to consume its data,
 * it must create a copy and instruct Rust to destroy it via the
 * [`destroy_unmanaged_vector`] FFI export.
 *
 * An UnmanagedVector is immutable.
 *
 * ## Ownership
 *
 * Ownership is the right and the obligation to destroy an `UnmanagedVector`
 * exactly once. Both Rust and Go can create an `UnmanagedVector`, which gives
 * then ownership. Sometimes it is necessary to transfer ownership.
 *
 * ### Transfer ownership from Rust to Go
 *
 * When an `UnmanagedVector` was created in Rust using [`UnmanagedVector::new`], [`UnmanagedVector::default`]
 * or [`new_unmanaged_vector`], it can be passted to Go as a return value.
 * Rust then has no chance to destroy the vector anymore, so ownership is transferred to Go.
 * In Go, the data has to be copied to a garbage collected `[]byte`. Then the vector must be destroyed
 * using [`destroy_unmanaged_vector`].
 *
 * ### Transfer ownership from Go to Rust
 *
 * When Rust code calls into Go (using the vtable methods), return data or error messages must be created
 * in Go. This is done by calling [`new_unmanaged_vector`] from Go, which copies data into a newly created
 * `UnmanagedVector`. Since Go created it, it owns it. The ownership is then passed to Rust via the
 * mutable return value pointers. On the Rust side, the vector is destroyed using [`UnmanagedVector::consume`].
 *
 */
typedef struct {
  /**
   * True if and only if this is None. If this is true, the other fields must be ignored.
   */
  bool is_none;
  uint8_t *ptr;
  size_t len;
  size_t cap;
} UnmanagedVector;

/**
 * A view into an externally owned byte slice (Go `[]byte`).
 * Use this for the current call only. A view cannot be copied for safety reasons.
 * If you need a copy, use [`ByteSliceView::to_owned`].
 *
 * Go's nil value is fully supported, such that we can differentiate between nil and an empty slice.
 */
typedef struct {
  /**
   * True if and only if the byte slice is nil in Go. If this is true, the other fields must be ignored.
   */
  bool is_nil;
  const uint8_t *ptr;
  size_t len;
} ByteSliceView;

typedef struct {
  uint8_t _private[0];
} db_t;

/**
 * A view into a `Option<&[u8]>`, created and maintained by Rust.
 *
 * This can be copied into a []byte in Go.
 */
typedef struct {
  /**
   * True if and only if this is None. If this is true, the other fields must be ignored.
   */
  bool is_none;
  const uint8_t *ptr;
  size_t len;
} U8SliceView;

typedef struct {
  /**
   * An ID assigned to this contract call
   */
  uint64_t call_id;
  uint64_t iterator_index;
} iterator_t;

typedef struct {
  int32_t (*next_db)(iterator_t, UnmanagedVector*, UnmanagedVector*);
} Iterator_vtable;

typedef struct {
  iterator_t state;
  Iterator_vtable vtable;
  size_t prefix_len;
} GoIter;

typedef struct {
  int32_t (*read_db)(db_t*, U8SliceView, UnmanagedVector*, UnmanagedVector*);
  int32_t (*write_db)(db_t*, U8SliceView, U8SliceView, UnmanagedVector*);
  int32_t (*remove_db)(db_t*, U8SliceView, UnmanagedVector*);
  int32_t (*scan_db)(db_t*,
                     U8SliceView,
                     U8SliceView,
                     U8SliceView,
                     int32_t,
                     GoIter*,
                     UnmanagedVector*);
} Db_vtable;

typedef struct {
  db_t *state;
  Db_vtable vtable;
} Db;

typedef struct {
  uint8_t _private[0];
} api_t;

typedef struct {
  int32_t (*query)(const api_t*,
                   U8SliceView,
                   uint64_t,
                   UnmanagedVector*,
                   uint64_t*,
                   UnmanagedVector*);
  int32_t (*get_account_info)(const api_t*,
                              U8SliceView,
                              bool*,
                              uint64_t*,
                              uint64_t*,
                              uint8_t*,
                              bool*,
                              UnmanagedVector*);
  int32_t (*amount_to_share)(const api_t*,
                             U8SliceView,
                             U8SliceView,
                             uint64_t,
                             uint64_t*,
                             UnmanagedVector*);
  int32_t (*share_to_amount)(const api_t*,
                             U8SliceView,
                             U8SliceView,
                             uint64_t,
                             uint64_t*,
                             UnmanagedVector*);
  int32_t (*unbond_timestamp)(const api_t*, uint64_t*, UnmanagedVector*);
  int32_t (*get_price)(const api_t*,
                       U8SliceView,
                       UnmanagedVector*,
                       uint64_t*,
                       uint64_t*,
                       UnmanagedVector*);
} GoApi_vtable;

typedef struct {
  const api_t *state;
  GoApi_vtable vtable;
} GoApi;

vm_t *allocate_vm(size_t module_cache_capacity, size_t script_cache_capacity);

UnmanagedVector convert_module_name(UnmanagedVector *errmsg,
                                    ByteSliceView precompiled,
                                    ByteSliceView module_name);

UnmanagedVector decode_module_bytes(UnmanagedVector *errmsg, ByteSliceView module_bytes);

UnmanagedVector decode_move_resource(Db db,
                                     UnmanagedVector *errmsg,
                                     ByteSliceView struct_tag,
                                     ByteSliceView resource_bytes);

UnmanagedVector decode_move_value(Db db,
                                  UnmanagedVector *errmsg,
                                  ByteSliceView type_tag,
                                  ByteSliceView value_bytes);

UnmanagedVector decode_script_bytes(UnmanagedVector *errmsg, ByteSliceView script_bytes);

void destroy_unmanaged_vector(UnmanagedVector v);

UnmanagedVector execute_contract(vm_t *vm_ptr,
                                 Db db,
                                 GoApi api,
                                 ByteSliceView env_payload,
                                 uint64_t gas_limit,
                                 ByteSliceView senders,
                                 ByteSliceView entry_function_payload,
                                 UnmanagedVector *errmsg);

UnmanagedVector execute_script(vm_t *vm_ptr,
                               Db db,
                               GoApi api,
                               ByteSliceView env_payload,
                               uint64_t gas_limit,
                               ByteSliceView senders,
                               ByteSliceView script_payload,
                               UnmanagedVector *errmsg);

UnmanagedVector execute_view_function(vm_t *vm_ptr,
                                      Db db,
                                      GoApi api,
                                      ByteSliceView env_payload,
                                      uint64_t gas_limit,
                                      ByteSliceView view_function_payload,
                                      UnmanagedVector *errmsg);

UnmanagedVector initialize(vm_t *vm_ptr,
                           Db db,
                           GoApi api,
                           ByteSliceView env_payload,
                           ByteSliceView module_bundle_payload,
                           ByteSliceView allowed_publishers_payload,
                           UnmanagedVector *errmsg);

UnmanagedVector new_unmanaged_vector(bool nil, const uint8_t *ptr, size_t length);

UnmanagedVector parse_struct_tag(UnmanagedVector *errmsg, ByteSliceView struct_tag_str);

UnmanagedVector read_module_info(UnmanagedVector *errmsg, ByteSliceView compiled);

void release_vm(vm_t *vm);

UnmanagedVector stringify_struct_tag(UnmanagedVector *errmsg, ByteSliceView struct_tag);

#endif /* __LIBMOVEVM__ */
