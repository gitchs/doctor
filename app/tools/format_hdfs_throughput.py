#!/usr/bin/env python3
import json
import sqlite3
from argparse import ArgumentParser


def main():
    cli_parser = ArgumentParser()
    cli_parser.add_argument('--db', required=True)
    cli_parser.add_argument('--output', default='/dev/stdout', required=False)
    cli_configure = cli_parser.parse_args()
    sql = '''select query_id,STRFTIME('%s', DATETIME(`start_time`)) start_ts, hdfs_statics from m2 where hdfs_statics != '{}' AND `duration`>10000 '''
    with open(cli_configure.output, 'w') as fd:
        conn = sqlite3.connect(cli_configure.db)
        cursor = conn.execute(sql)
        for row in cursor:
            query_id = row[0]
            start_ts = row[1]
            statics = json.loads(row[2])
            for oid, op in statics.items():
                line = f'{start_ts},{query_id},{oid},{op["min_throughtput"]:.2f}\n'
                fd.write(line)


if __name__ == '__main__':
    main()
