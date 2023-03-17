#ifndef EXTERNAL_D2_DBUTILS_H
#define EXTERNAL_D2_DBUTILS_H
#include <memory>
#include "d2.h"
#include "sqlite3.h"
#include "RuntimeProfile_types.h"

int InitDatabaseTables(sqlite3* conn);
int InsertQueryBasicInfo(sqlite3* conn, const ProfileAnalyzeResult& result);
int InsertQuerySkewOperatorInfo(sqlite3* conn, const ProfileAnalyzeResult& result, const std::shared_ptr<impala::TRuntimeProfileTree>& tree);



#endif  // EXTERNAL_D2_DBUTILS_H
