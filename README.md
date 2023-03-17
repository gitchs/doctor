## Standard
- C++17, life is short, make it easier.

## Deps
| 依赖 | 用途 |
|------|------|
| third_party/xxHash-0.8.1 | 快速哈希函数，用于比对相同的SQL出现频次 |
| third_party/cjson | lua json库 |
| third_party/luasql | lua的sliqte、mariadb等关系型数据库的封装 |
| third_party/sqlite-amalgamation-3390000 | 保存分析结果 |
| third_party/re2-2022-06-01 | 向lua runtime提供正则表达式能力，便于编写策略 |
| third_party/linenoise | readlined的替代品 |
| third_party/spdlog-1.11.0 | 日志库 |
| third_party/thrift-0.16.0 | 用于解析impala profile |
| third_party/impala-codegen | impala codegen代码，来自 https://github.com/apache/impala/tree/master/common/thrift |
| third_party/avro-cpp-1.11.0 | impala profile存储格式 |
| third_party/snappy-1.1.9 | avro依赖 |

## 核心文件

| 文件 | 描述 |
|------|------|
| app/test.lua | 读取avro文件，批量分析impala profile |
| app/strategies.lua | lua分析策略文件 |

