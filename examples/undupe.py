#!/usr/bin/env python3

# This script is meant to be executed as an external script through the
# cbftp browse screen. It will run "site undupe" on files with
# -missing suffix

import cbapi
import sys

api_token = sys.argv[1]
operation = sys.argv[2]

cbapi.init(api_token)

def undupe(site, path, filename):
  req("raw", {
    "sites": site,
    "path": path,
    "command": "site undupe " + filename,
    "timeout": 10
  })


if operation.startswith("browse-site"):
  site = sys.argv[3]
  path = sys.argv[4]
  items = sys.argv[5].split(",")
  base_list = cbapi.req(f"path?site={site}&path={path}")
  undupe_list = []
  for file in base_list:
    if file['type'] == "FILE":
      if file['name'].endswith('-missing'):
        undupe_list.append(file['name'].replace('-missing',''))
  if not undupe_list:
    print("No files to undupe...")
    cbapi.exit(0)
  print(f"Unduping {len(undupe_list)} missing files on {site}...")
  sys.stdout.flush()
  for missing_file in undupe_list:
    undupe(site, path, missing_file)
  print("Undupe complete!")
  cbapi.exit(0)
else:
  print(f"Unsupported operation: {operation}")
  cbapi.exit(1)
