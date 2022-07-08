#!/usr/bin/env python3
import os
import json
import sys
import logging
from argparse import ArgumentParser
from rich.tree import Tree
import rich

def gen_label(index, node):
    return f'{index} {node["name"]}'

def build_tree(parent, nodes, index):
    node = nodes[index]
    vnode = None
    offset = 1
    label = gen_label(index, node)
    if parent is None:
        parent = Tree(label)
        vnode = parent
    else:
        vnode = parent.add(label)
    if node['counters']:
        cnode = vnode.add('[COUNTERS]')
        for k, v in node['counters'].items():
            cnode.add(f'{k} {v}')
    if node['num_children'] > 0:
        for _ in range(node['num_children']):
            delta, _ = build_tree(vnode, nodes, index+offset)
            offset += delta
    return offset, vnode

def main():
    logging.basicConfig(
        level=logging.INFO,
        format='[%(asctime)s] %(filename)s:%(lineno)d %(message)s'
    )
    cli_parser = ArgumentParser()
    cli_parser.add_argument('--input', '-i', required=False)
    cli_configure = cli_parser.parse_args()
    rows = []
    if os.isatty(0):
        assert(cli_configure.input is not None)
        with open(cli_configure.input, 'r') as fd:
            for line in fd:
                line = line.strip()
                if not line:
                    continue
                rows.append(json.loads(line))
    else:
        rows = json.loads(sys.stdin.read())
    n, root = build_tree(None, rows, 0)
    assert(n == len(rows))
    rich.print(root)

if __name__ == '__main__':
    main()
