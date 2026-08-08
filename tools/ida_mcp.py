"""Minimal MCP client for the IDA Pro MCP server (streamable HTTP).

Usage:
    python tools/ida_mcp.py <tool_name> ['{"arg": "value", ...}']

Examples:
    python tools/ida_mcp.py tools/list
    python tools/ida_mcp.py decompile '{"addr": "0x84B760"}'
    python tools/ida_mcp.py disasm '{"addr": "0x84B760", "max_instructions": 80}'
"""

import asyncio
import json
import sys

import anyio
from mcp import ClientSession
from mcp.client.streamable_http import streamable_http_client


MCP_URL = "http://127.0.0.1:13337/mcp"


async def run(tool_name: str, arguments: dict):
    async with streamable_http_client(MCP_URL) as (read, write):
        try:
            async with ClientSession(read, write) as session:
                await session.initialize()
                result = await session.call_tool(tool_name, arguments)
                for content in result.content:
                    if getattr(content, "type", None) == "text":
                        print(content.text)
                    else:
                        print(content)
                if getattr(result, "is_error", False) or getattr(result, "isError", False):
                    sys.exit(2)
        except anyio.get_cancelled_exc_class():
            raise
        except BaseException as exc:
            # The server may not implement session termination; that's fine.
            if "501" not in str(exc):
                raise


def main() -> int:
    if len(sys.argv) < 2:
        print(__doc__)
        return 1
    tool_name = sys.argv[1]
    if len(sys.argv) > 2:
        arguments = json.loads(sys.argv[2])
    else:
        arguments = json.load(sys.stdin) if not sys.stdin.isatty() else {}
    asyncio.run(run(tool_name, arguments))
    return 0


if __name__ == "__main__":
    sys.exit(main())
