#!/usr/bin/env python3

# This script is meant to be executed as an external script through the
# cbftp browse screen. It will run "site rescan" in the currently
# selected directory, or in the current directory if a file is selected.

import cbapi
import sys

api_token = sys.argv[1]
operation = sys.argv[2]

cbapi.init(api_token)

def rescan(site, path):
  cbapi.req("raw", {
    "sites": site,
    "path": path,
    "command": "site rescan",
    "timeout": 600
  })

if operation.startswith("browse-site"):
  site = sys.argv[3]
  path = sys.argv[4]
  items = sys.argv[5].split(",")
  base_list = cbapi.req(f"path?site={site}&path={path}")
  rescan_base = False
  rescan_list = []
  for item in items:
    for file in base_list:
      if file['name'] == item:
        if file['type'] == "DIR":
          rescan_list.append(f"{path}/{item}")
        elif file['type'] == "LINK":
          rescan_list.append(file['link_target'])
        else:
          rescan_base = True
        break
  if rescan_base:
    rescan_list.append(path)
  if len(rescan_list) == 1:
    print(f"Rescanning {rescan_list[0]} on {site}...")
  else:
    print(f"Rescanning {len(rescan_list)} items on {site}...")
  sys.stdout.flush()
  for rescan_item in rescan_list:
    rescan(site, rescan_item)
  print("Rescan complete! lulz")
  cbapi.exit(0)
else:
  print(f"Unsupported operation: {operation}")
  cbapi.exit(1)
