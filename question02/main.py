#!/usr/bin/env python3
import json
import sys
import urllib.request
from functools import cmp_to_key
from typing import NoReturn, List, Dict, Tuple


def api_access(monster_pair: List[str]) -> Dict[str, str]:
    url = f'https://ob6la3c120.execute-api.ap-northeast-1.amazonaws.com/Prod/battle/{monster_pair[0]}+{monster_pair[1]}'
    req = urllib.request.Request(url)
    with urllib.request.urlopen(req) as res:
        body = res.read().decode('utf-8')
        return json.loads(body)


def monster_pairs(monsters: List[str]) -> List[List[str]]:
    combinations = []
    for x in range(len(monsters) - 1):
        for y in range(x + 1, len(monsters)):
            combinations.append([monsters[x], monsters[y]])

    return combinations


def create_rank_table(monsters: List[str]) -> Dict[Tuple[str, str], bool]:
    table : Dict[Tuple[str, str], bool]= {}
    pairs = monster_pairs(monsters)
    for pair in pairs:
        res = api_access(pair)
        winner, loser = res['winner'], res['loser']
        print(f'Winner {winner}, Loser {loser}')
        table[(winner, loser)] = True

    return table


def main() -> NoReturn:
    if len(sys.argv) < 2:
        print('Usage: main.py monsters...', file=sys.stderr)
        sys.exit(1)

    monsters = sys.argv[1:]
    rank_table = create_rank_table(monsters)

    def sort_func(a: str, b: str) -> int:
        if (a, b) in rank_table:
            return -1
        if (b, a) in rank_table:
            return 1
        return 0

    sorted_monsters = sorted(monsters, key=cmp_to_key(sort_func))
    print(sorted_monsters)


if __name__ == '__main__':
    main()
