DROP VIEW IF EXISTS m2;
CREATE TABLE IF NOT EXISTS m2
AS
SELECT
query_id
,skew
,JSON_EXTRACT(summary_info_strings, '$."Query Type"') AS query_type
,JSON_EXTRACT(summary_info_strings, '$."Start Time"') AS start_time
,JSON_EXTRACT(summary_info_strings, '$."End Time"') AS end_time
,CAST(JSON_EXTRACT(summary_info_strings, '$."Duration(ms)"') AS INT) AS duration_ms
,CAST(JSON_EXTRACT(impala_server_counter, '$."ClientFetchWaitTimer"')/1e6 AS INT) AS `client_fetch_wait_timer_ms`
,CAST(JSON_EXTRACT(summary_info_strings, '$."Admission Wait"') AS INT) AS admission_wait
,CAST(JSON_EXTRACT(summary_info_strings, '$."Estimated Per-Host Mem"') AS INT) AS estimated_per_host_memory
,CAST(JSON_EXTRACT(eprofile_info_strings , '$."Peak Memory Usage"') AS INT) AS peak_memory_usage
,JSON_EXTRACT(summary_info_strings, '$."Network Address"') AS network_address
,JSON_EXTRACT(summary_info_strings, '$."Query State"') AS query_state
,JSON_EXTRACT(summary_info_strings, '$."Query Status"') AS query_status
,JSON_EXTRACT(summary_info_strings, '$."User"') AS `user`
,JSON_EXTRACT(summary_info_strings, '$."Default Db"') AS default_db
,JSON_EXTRACT(summary_info_strings, '$."Coordinator"') AS coordinator
,JSON_EXTRACT(summary_info_strings, '$."Errors"') AS summary_errors
,JSON_EXTRACT(summary_info_strings, '$."ExecSummary"') AS summary
,JSON_EXTRACT(summary_info_strings, '$."Plan"') AS query_plan
,CAST(JSON_EXTRACT(eprofile_counters, '$."TotalCpuTime"')/1e6 AS INT) AS total_cpu_time_ms
,CAST(JSON_EXTRACT(eprofile_counters, '$."TotalBytesRead"') AS INT) AS total_bytes_read
,CAST(JSON_EXTRACT(eprofile_counters, '$."TotalBytesSent"') AS INT) AS total_bytes_sent
,CAST(JSON_EXTRACT(eprofile_counters, '$."TotalInnerBytesSent"') AS INT) AS total_inner_bytes_sent
,CAST(JSON_EXTRACT(eprofile_counters, '$."TotalScanBytesSent"') AS INT) AS total_scan_bytes_sent
,CAST(JSON_EXTRACT(eprofile_counters, '$."NumBackends"') AS INT) AS num_backends
,JSON_EXTRACT(eprofile_info_strings, '$."Per Node System Time"') AS per_node_sys_time
,JSON_EXTRACT(eprofile_info_strings, '$."Per Node User Time"') AS per_node_user_time
,JSON_EXTRACT(eprofile_info_strings, '$."Per Node Bytes Read"') AS per_node_bytes_read
,JSON_EXTRACT(summary_info_strings, '$."Sql Statement"') AS `sql`
FROM basic ;
CREATE INDEX IF NOT EXISTS idx_m2_query_id ON m2(query_id);
CREATE INDEX IF NOT EXISTS idx_m2_query_type ON m2(query_type);
CREATE INDEX IF NOT EXISTS idx_m2_start_time ON m2(start_time);
CREATE INDEX IF NOT EXISTS idx_m2_duration ON m2(duration_ms);
CREATE INDEX IF NOT EXISTS idx_m2_admission_wait ON m2(admission_wait);

CREATE VIEW IF NOT EXISTS skew_hdfs_ops
AS
select
query_id
,fragment
,substring(substring(host, 50), 0, length(host) - 49) AS `host`
,name
,ROUND(JSON_EXTRACT(counters, '$.BytesRead') / 1048576.0, 2) AS MBRead
,ROUND(JSON_EXTRACT(counters, '$.BytesReadLocal') / 1048576.0, 2) AS MBReadLocal
,ROUND(JSON_EXTRACT(counters, '$.BytesReadShortCircuit') / 1048576.0, 2) AS MBReadShortCircuit
,JSON_EXTRACT(counters, '$.RowsRead') AS RowsRead
,JSON_EXTRACT(counters, '$.RowsReturned') AS RowsReturned
,JSON_EXTRACT(counters, '$."TotalTime"')/1e9 AS TotalTimeSec
,JSON_EXTRACT(counters, '$.ScannerIoWaitTime')/1e6 AS ScannerIoWaitTime
,JSON_EXTRACT(counters, '$."TotalRawHdfsOpenFileTime(*)"')/1e6 AS TotalRawHdfsOpenFileTime
,JSON_EXTRACT(counters, '$."TotalRawHdfsReadTime(*)"')/1e6 AS TotalRawHdfsReadTime
,ROUND(JSON_EXTRACT(counters, '$."TotalReadThroughput"')/1048576.0, 2) AS TotalReadThroughputMB
,JSON_EXTRACT(counters, '$."PeakMemoryUsage"') AS PeakMemoryUsage
,JSON_EXTRACT(counters, '$."DecompressionTime"')/1e6 AS DecompressionTime
,JSON_EXTRACT(counters, '$."NumDisksAccessed"') AS NumDisksAccessed
from ops
WHERE 1=1
	AND `name` LIKE 'HDFS_SCAN_NODE%'
;
