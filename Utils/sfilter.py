import sys
import json
import jmespath

if len(sys.argv) < 3:
    print(f'Usage: {sys.argv[0]} <catalog> <predicate>')
    sys.exit(1)

predicate = sys.argv[2].replace("'", "`")

try:
    with open(sys.argv[1], 'r') as f:
        jsonObj = json.load(f)
    
    expr = jmespath.compile(f'sources[*].islands[?{predicate}].iau_name[]')
    res = expr.search(jsonObj)
    for s in res:
        print(s)

except Exception as e:
    print('Parse Error!\n', e)
    sys.exit(1)

