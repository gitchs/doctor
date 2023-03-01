#!/usr/bin/env python3
"""
parallel.py会生成很多sqlite-$PID.db数据库，
为了方便分析,需要将他们合并
"""
import time
import logging
from argparse import ArgumentParser
import sqlite3


def main():
    "main entry"
    logging.basicConfig(
        level=logging.INFO,
        format='[%(asctime)s] %(filename)s:%(lineno)d %(message)s'
    )
    cli_parser = ArgumentParser()
    cli_parser.add_argument('--db', nargs='+', required=True)
    cli_parser.add_argument('--output', '-o', default=f'profile-merged-{int(time.time())}.db')
    cli_configure = cli_parser.parse_args()
    db = sqlite3.connect(cli_configure.output)
    detach_sql = "DETACH DATABASE src_db"
    if len(cli_configure.db) > 1:
        db_filename = cli_configure.db[0]
        attach_sql = f"ATTACH DATABASE '{db_filename}' AS src_db"
        ctas_sql0 = "CREATE TABLE m2 AS SELECT * FROM src_db.m2"
        ctas_sql1 = "CREATE TABLE profile AS SELECT * FROM src_db.profile"
        db.execute(attach_sql)
        db.execute(ctas_sql0)
        db.execute(ctas_sql1)
        db.commit()
        db.execute(detach_sql)
    for db_filename in cli_configure.db[1:]:
        attach_sql = f"ATTACH DATABASE '{db_filename}' AS src_db"
        insert_sql0 = "INSERT INTO m2 SELECT * FROM src_db.m2"
        insert_sql1 = "INSERT INTO profile SELECT * FROM src_db.profile"
        db.execute(attach_sql)
        db.execute(insert_sql0)
        db.execute(insert_sql1)
        db.commit()
        db.execute(detach_sql)
    db.execute("CREATE INDEX m2_query_id ON m2(query_id)")
    db.execute("CREATE INDEX m2_query_id_is_slow ON m2(query_id, is_slow)")
    db.execute("""
CREATE INDEX m2_query_id_is_slow_start_time
ON m2(query_id, is_slow, start_time)""")
    db.execute("""
CREATE INDEX profile_query_id
ON profile(query_id)""")
    db.execute('''CREATE VIEW mview AS
SELECT query_id
,query_type
,query_state
,query_status
,coordinator
,rows_produced
,is_slow
,has_skew_ops
,start_time
,end_time
,duration
,admission_wait
,hdfs_statics
,result FROM m2''')
    db.commit()

if __name__ == '__main__':
    main()
