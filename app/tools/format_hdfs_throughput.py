#!/usr/bin/env python3
"format each query's hdfs min throughput"
import json
import sqlite3
from argparse import ArgumentParser


def main():
    "main entry"
    cli_parser = ArgumentParser()
    cli_parser.add_argument('--db', required=True)
    cli_parser.add_argument('--output', default='/dev/stdout', required=False)
    cli_configure = cli_parser.parse_args()
    sql = '''
SELECT 
    query_id,
    STRFTIME('%s', DATETIME(`start_time`)) start_ts,
    hdfs_statics
FROM m2
WHERE hdfs_statics != '{}' AND `duration`>10000 '''
    with open(cli_configure.output, 'w', encoding='utf8') as fd:
        fd.write('ts,query_id,oid,min_throughput,max_throughput,sum_bytes_read\n')
        conn = sqlite3.connect(cli_configure.db)
        cursor = conn.execute(sql)
        for row in cursor:
            query_id = row[0]
            start_ts = row[1]
            statics = json.loads(row[2])
            for oid, op in statics.items(): # pylint: disable=C0103
                line = f'{start_ts},{query_id},{oid},{op["min_throughput"]:.2f},'\
                   f'{op["max_throughput"]:.2f},{op["sum_bytes_read"]:.2f}\n'
                fd.write(line)


if __name__ == '__main__':
    main()
