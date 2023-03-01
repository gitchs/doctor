#!/usr/bin/env python3
"""test.lua是单线程，并没有提供任何并发的能力。
为了提高吞吐，使用python包一层，通过多进程分发文件处理
"""
from argparse import ArgumentParser
import logging
from pathlib import Path
from multiprocessing.pool import ThreadPool
from subprocess import Popen
from uuid import uuid4


def task(task_configure) -> int:
    "开子进程，处理任务"
    batch_id, f = task_configure
    f = f.absolute()
    database_filename = f'sqlite-{batch_id}-{uuid4().bytes.hex()}.db'
    logging.info('handle file "%s", output %s', f, database_filename)
    child_args = [
        str(Path(__file__).parent.joinpath('doctor').absolute()),
        str(Path(__file__).parent.joinpath('test.lua').absolute()),
        '-p',
        '-f', str(f),
        '-d', database_filename,
    ]
    with Popen(child_args, cwd=str(Path(__file__).parent.absolute())) as child:
        return child.wait()


def fit(dirs):
    "文件任务队列iterator"
    batch_id = uuid4().bytes.hex()
    for d in dirs:
        for f in Path(d).glob('*.avro'):
            yield (batch_id, f)

def main():
    "main entry"
    logging.basicConfig(
        level=logging.INFO,
        format='[%(asctime)s] %(filename)s:%(lineno)d %(message)s'
    )
    cli_parser = ArgumentParser()
    cli_parser.add_argument('--root', '-r', nargs='+', required=True)
    cli_parser.add_argument('--bin', required=False, default='./doctor')
    cli_parser.add_argument('--jobs', '-j', default='6', type=int, required=False)
    cli_configure = cli_parser.parse_args()
    with ThreadPool(cli_configure.jobs) as p:
        p.map(task, fit(cli_configure.root))

if __name__ == '__main__':
    main()
