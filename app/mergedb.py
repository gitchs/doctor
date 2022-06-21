#!/usr/bin/env python3
import logging
from argparse import ArgumentParser
import sqlite3
from pathlib import Path

def main():
    logging.basicConfig(
        level=logging.INFO,
        format='[%(asctime)s] %(filename)s:%(lineno)d %(message)s'                            # easy format
    )
    cli_parser = ArgumentParser()
    cli_parser.add_argument('--db', nargs='+', required=True)
    cli_parser.add_argument('--output', '-o', default='profile-merged.db')
    cli_configure = cli_parser.parse_args()
    db = sqlite3.connect(cli_configure.output)
    select_sqls = []
    for index in range(len(cli_configure.db)):
        db_name = 'db%02d' % index
        db_filename = cli_configure.db[index]
        attach_sql = f"ATTACH DATABASE '{db_filename}' AS {db_name}"
        select_sql = f"SELECT * FROM {db_name}.m2"
        select_sqls.append(select_sql)
        db.execute(attach_sql)
    ctas_sql = "CREATE TABLE m2 AS %s" % ('\nUNION ALL\n'.join(select_sqls))
    db.execute(ctas_sql)
    db.commit()
    db.execute("CREATE INDEX m2_query_id ON m2(query_id)")
    db.execute("CREATE INDEX m2_query_id_is_slow ON m2(query_id, is_slow)")
    db.execute("CREATE INDEX m2_query_id_is_slow_start_time ON m2(query_id, is_slow, start_time)")
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
,result FROM m2''')
    db.commit()

if __name__ == '__main__':
    main()
