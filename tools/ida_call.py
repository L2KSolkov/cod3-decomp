import json, sys, urllib.request

def call(tool, args):
    body = json.dumps({"jsonrpc":"2.0","id":1,"method":"tools/call","params":{"name":tool,"arguments":args}}).encode()
    req = urllib.request.Request("http://127.0.0.1:13337/mcp", data=body, headers={"Content-Type":"application/json"})
    with urllib.request.urlopen(req, timeout=300) as r:
        return json.loads(r.read())

if __name__ == "__main__":
    tool = sys.argv[1]
    args = json.load(sys.stdin) if not sys.stdin.isatty() else {}
    res = call(tool, args)
    print(json.dumps(res))
