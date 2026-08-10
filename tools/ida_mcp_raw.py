"""Minimal stdlib MCP client for the IDA Pro MCP server (streamable HTTP).

Drop-in replacement for ida_mcp.py when the `mcp`/pydantic packages are
unavailable (e.g. blocked by Application Control policies).

Usage:
    python tools/ida_mcp_raw.py <tool_name> ['{"arg": "value", ...}']

Example:
    python tools/ida_mcp_raw.py decompile '{"addr": "0x62C2D0"}'
"""

import json
import sys
import urllib.request

MCP_URL = "http://127.0.0.1:13337/mcp"


def post(payload, session_id=None):
    headers = {
        "Accept": "application/json, text/event-stream",
        "Content-Type": "application/json",
    }
    if session_id is not None:
        headers["Mcp-Session-Id"] = session_id
    req = urllib.request.Request(
        MCP_URL, data=json.dumps(payload).encode("utf-8"), headers=headers
    )
    with urllib.request.urlopen(req) as resp:
        return resp.read().decode("utf-8"), resp.headers.get("Mcp-Session-Id")


def main():
    if len(sys.argv) < 2:
        print(__doc__)
        return 1
    tool_name = sys.argv[1]
    if len(sys.argv) > 2:
        arguments = json.loads(sys.argv[2])
    else:
        arguments = json.load(sys.stdin) if not sys.stdin.isatty() else {}

    body, session_id = post(
        {
            "jsonrpc": "2.0",
            "id": 1,
            "method": "initialize",
            "params": {
                "protocolVersion": "2025-03-26",
                "capabilities": {},
                "clientInfo": {"name": "ida_mcp_raw", "version": "1.0"},
            },
        }
    )
    body, session_id = post(
        {
            "jsonrpc": "2.0",
            "id": 2,
            "method": "tools/call",
            "params": {"name": tool_name, "arguments": arguments},
        },
        session_id,
    )
    result = json.loads(body)
    if "error" in result:
        print(result["error"])
        return 2
    for content in result.get("result", {}).get("content", []):
        if content.get("type") == "text":
            print(content["text"])
        else:
            print(content)
    return 0


if __name__ == "__main__":
    sys.exit(main())
