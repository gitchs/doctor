/**
 * Autogenerated by Thrift Compiler (0.16.0)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
#ifndef ErrorCodes_TYPES_H
#define ErrorCodes_TYPES_H

#include <iosfwd>

#include <thrift/Thrift.h>
#include <thrift/TApplicationException.h>
#include <thrift/TBase.h>
#include <thrift/protocol/TProtocol.h>
#include <thrift/transport/TTransport.h>

#include <functional>
#include <memory>


namespace impala {

struct TErrorCode {
  enum type {
    OK = 0,
    UNUSED = 1,
    GENERAL = 2,
    CANCELLED = 3,
    ANALYSIS_ERROR = 4,
    NOT_IMPLEMENTED_ERROR = 5,
    RUNTIME_ERROR = 6,
    MEM_LIMIT_EXCEEDED = 7,
    INTERNAL_ERROR = 8,
    RECOVERABLE_ERROR = 9,
    PARQUET_MULTIPLE_BLOCKS = 10,
    PARQUET_COLUMN_METADATA_INVALID = 11,
    PARQUET_HEADER_PAGE_SIZE_EXCEEDED = 12,
    PARQUET_HEADER_EOF = 13,
    PARQUET_GROUP_ROW_COUNT_ERROR = 14,
    PARQUET_GROUP_ROW_COUNT_OVERFLOW = 15,
    PARQUET_MISSING_PRECISION = 16,
    PARQUET_WRONG_PRECISION = 17,
    PARQUET_BAD_CONVERTED_TYPE = 18,
    PARQUET_INCOMPATIBLE_DECIMAL = 19,
    SEQUENCE_SCANNER_PARSE_ERROR = 20,
    SNAPPY_DECOMPRESS_INVALID_BLOCK_SIZE = 21,
    SNAPPY_DECOMPRESS_INVALID_COMPRESSED_LENGTH = 22,
    SNAPPY_DECOMPRESS_UNCOMPRESSED_LENGTH_FAILED = 23,
    SNAPPY_DECOMPRESS_RAW_UNCOMPRESS_FAILED = 24,
    SNAPPY_DECOMPRESS_DECOMPRESS_SIZE_INCORRECT = 25,
    FRAGMENT_EXECUTOR = 26,
    PARTITIONED_HASH_JOIN_MAX_PARTITION_DEPTH = 27,
    PARTITIONED_AGG_MAX_PARTITION_DEPTH = 28,
    MISSING_BUILTIN = 29,
    RPC_GENERAL_ERROR = 30,
    RPC_RECV_TIMEOUT = 31,
    UDF_VERIFY_FAILED = 32,
    PARQUET_CORRUPT_RLE_BYTES = 33,
    AVRO_DECIMAL_RESOLUTION_ERROR = 34,
    AVRO_DECIMAL_METADATA_MISMATCH = 35,
    AVRO_SCHEMA_RESOLUTION_ERROR = 36,
    AVRO_SCHEMA_METADATA_MISMATCH = 37,
    AVRO_UNSUPPORTED_DEFAULT_VALUE = 38,
    AVRO_MISSING_FIELD = 39,
    AVRO_MISSING_DEFAULT = 40,
    AVRO_NULLABILITY_MISMATCH = 41,
    AVRO_NOT_A_RECORD = 42,
    PARQUET_DEF_LEVEL_ERROR = 43,
    PARQUET_NUM_COL_VALS_ERROR = 44,
    PARQUET_DICT_DECODE_FAILURE = 45,
    SSL_PASSWORD_CMD_FAILED = 46,
    SSL_CERTIFICATE_PATH_BLANK = 47,
    SSL_PRIVATE_KEY_PATH_BLANK = 48,
    SSL_CERTIFICATE_NOT_FOUND = 49,
    SSL_PRIVATE_KEY_NOT_FOUND = 50,
    SSL_SOCKET_CREATION_FAILED = 51,
    MEM_ALLOC_FAILED = 52,
    PARQUET_REP_LEVEL_ERROR = 53,
    PARQUET_UNRECOGNIZED_SCHEMA = 54,
    COLLECTION_ALLOC_FAILED = 55,
    TMP_DEVICE_BLACKLISTED = 56,
    TMP_FILE_BLACKLISTED = 57,
    RPC_CLIENT_CONNECT_FAILURE = 58,
    STALE_METADATA_FILE_TOO_SHORT = 59,
    PARQUET_BAD_VERSION_NUMBER = 60,
    SCANNER_INCOMPLETE_READ = 61,
    SCANNER_INVALID_READ = 62,
    AVRO_BAD_VERSION_HEADER = 63,
    UDF_MEM_LIMIT_EXCEEDED = 64,
    UNUSED_65 = 65,
    COMPRESSED_FILE_MULTIPLE_BLOCKS = 66,
    COMPRESSED_FILE_BLOCK_CORRUPTED = 67,
    COMPRESSED_FILE_DECOMPRESSOR_ERROR = 68,
    COMPRESSED_FILE_DECOMPRESSOR_NO_PROGRESS = 69,
    COMPRESSED_FILE_TRUNCATED = 70,
    DATASTREAM_SENDER_TIMEOUT = 71,
    KUDU_IMPALA_TYPE_MISSING = 72,
    IMPALA_KUDU_TYPE_MISSING = 73,
    KUDU_NOT_SUPPORTED_ON_OS = 74,
    KUDU_NOT_ENABLED = 75,
    PARTITIONED_HASH_JOIN_REPARTITION_FAILS = 76,
    UNUSED_77 = 77,
    AVRO_TRUNCATED_BLOCK = 78,
    AVRO_INVALID_UNION = 79,
    AVRO_INVALID_BOOLEAN = 80,
    AVRO_INVALID_LENGTH = 81,
    SCANNER_INVALID_INT = 82,
    AVRO_INVALID_RECORD_COUNT = 83,
    AVRO_INVALID_COMPRESSED_SIZE = 84,
    AVRO_INVALID_METADATA_COUNT = 85,
    SCANNER_STRING_LENGTH_OVERFLOW = 86,
    PARQUET_CORRUPT_PLAIN_VALUE = 87,
    PARQUET_CORRUPT_DICTIONARY = 88,
    TEXT_PARSER_TRUNCATED_COLUMN = 89,
    SCRATCH_LIMIT_EXCEEDED = 90,
    BUFFER_ALLOCATION_FAILED = 91,
    PARQUET_ZERO_ROWS_IN_NON_EMPTY_FILE = 92,
    NO_REGISTERED_BACKENDS = 93,
    KUDU_KEY_ALREADY_PRESENT = 94,
    KUDU_NOT_FOUND = 95,
    KUDU_SESSION_ERROR = 96,
    AVRO_UNSUPPORTED_TYPE = 97,
    AVRO_INVALID_DECIMAL = 98,
    KUDU_NULL_CONSTRAINT_VIOLATION = 99,
    PARQUET_TIMESTAMP_OUT_OF_RANGE = 100,
    SCRATCH_ALLOCATION_FAILED = 101,
    SCRATCH_READ_TRUNCATED = 102,
    KUDU_TIMESTAMP_OUT_OF_RANGE = 103,
    MAX_ROW_SIZE = 104,
    IR_VERIFY_FAILED = 105,
    MINIMUM_RESERVATION_UNAVAILABLE = 106,
    ADMISSION_REJECTED = 107,
    ADMISSION_TIMED_OUT = 108,
    THREAD_CREATION_FAILED = 109,
    DISK_IO_ERROR = 110,
    DATASTREAM_RECVR_CLOSED = 111,
    BAD_PRINCIPAL_FORMAT = 112,
    LZ4_COMPRESSION_INPUT_TOO_LARGE = 113,
    SASL_APP_NAME_MISMATCH = 114,
    PARQUET_BIT_PACKED_LEVELS = 115,
    ROW_BATCH_TOO_LARGE = 116,
    LIB_VERSION_MISMATCH = 117,
    SCRATCH_READ_VERIFY_FAILED = 118,
    CANCELLED_INTERNALLY = 119,
    SERVER_SHUTTING_DOWN = 120,
    PARQUET_TIMESTAMP_INVALID_TIME_OF_DAY = 121,
    PARQUET_CORRUPT_BOOL_VALUE = 122,
    THREAD_POOL_SUBMIT_FAILED = 123,
    THREAD_POOL_TASK_TIMED_OUT = 124,
    UNREACHABLE_IMPALADS = 125,
    INACTIVE_SESSION_EXPIRED = 126,
    INACTIVE_QUERY_EXPIRED = 127,
    EXEC_TIME_LIMIT_EXCEEDED = 128,
    CPU_LIMIT_EXCEEDED = 129,
    SCAN_BYTES_LIMIT_EXCEEDED = 130,
    ROWS_PRODUCED_LIMIT_EXCEEDED = 131,
    EXPR_REWRITE_RESULT_LIMIT_EXCEEDED = 132,
    UNRESPONSIVE_BACKEND = 133,
    PARQUET_DATE_OUT_OF_RANGE = 134,
    DISCONNECTED_SESSION_CLOSED = 135,
    UNAUTHORIZED_SESSION_USER = 136,
    ZSTD_ERROR = 137,
    LZ4_BLOCK_DECOMPRESS_DECOMPRESS_SIZE_INCORRECT = 138,
    LZ4_BLOCK_DECOMPRESS_INVALID_INPUT_LENGTH = 139,
    LZ4_BLOCK_DECOMPRESS_INVALID_COMPRESSED_LENGTH = 140,
    LZ4_DECOMPRESS_SAFE_FAILED = 141,
    LZ4_COMPRESS_DEFAULT_FAILED = 142,
    MAX_STATEMENT_LENGTH_EXCEEDED = 143,
    AVRO_INVALID_DATE = 144,
    ORC_TIMESTAMP_OUT_OF_RANGE = 145,
    ORC_DATE_OUT_OF_RANGE = 146,
    ORC_NESTED_TYPE_MISMATCH = 147,
    ORC_TYPE_NOT_ROOT_AT_STRUCT = 148,
    NAAJ_OUT_OF_MEMORY = 149,
    INVALID_QUERY_HANDLE = 150,
    JOIN_ROWS_PRODUCED_LIMIT_EXCEEDED = 151,
    LOCAL_DISK_FAULTY = 152,
    JWKS_PARSE_ERROR = 153,
    JWT_VERIFY_FAILED = 154,
    PARQUET_ROWS_SKIPPING = 155
  };
};

extern const std::map<int, const char*> _TErrorCode_VALUES_TO_NAMES;

std::ostream& operator<<(std::ostream& out, const TErrorCode::type& val);

std::string to_string(const TErrorCode::type& val);

} // namespace

#endif
