#!/usr/bin/env python3
#
# This is a simple api script that watches a dir on a source site and starts
# transfer jobs for new items to the target site.
#

import cbapi
import datetime
import re
import time

cb_api_pass = 'your_api_password'

source_site = 'site1'
target_site = 'site2'
source_section_path = '/incoming/debian-isos'
target_section_path = '/DEBIAN'
match_pattern = "^.*-[A-Za-z0-9]+$"
check_interval_seconds = 300
max_backlog_seconds = 30000


transfered_items = dict()
pattern = re.compile(match_pattern)

def timestamp():
  return datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")

def get_existing_items():
  target_dir = cbapi.req(f"path?site={target_site}&path={target_section_path}")
  target_dir_existing_items = {}
  for item in target_dir:
    if item.get('type') != 'DIR':
      continue
    name = item.get('name')
    if not pattern.match(name):
      continue
    target_dir_existing_items[name] = True
  return target_dir_existing_items

def get_recent_source_items():
  now = datetime.datetime.now()
  items = []
  source_dir = cbapi.req(f"path?site={source_site}&path={source_section_path}")
  for item in source_dir:
    if item.get('type') != 'DIR':
      continue
    name = item.get('name')
    if not pattern.match(name):
      continue
    raw_timestamp = item.get('last_modified')
    parsed_timestamp = datetime.datetime.strptime(raw_timestamp, "%Y-%m-%d %H:%M")
    item_age = now - parsed_timestamp
    if item_age.seconds < max_backlog_seconds:
      items.append(name)
  return items

def mirror():
  target_dir_existing_items = None
  recent_items = get_recent_source_items()
  for name in recent_items:
    if name in transfered_items:
      continue
    transfered_items[name] = True
    if target_dir_existing_items == None:
      target_dir_existing_items = get_existing_items()
    if name in target_dir_existing_items:
      print(f"{timestamp()} - Exists: {name}")
      continue
    print(f"{timestamp()} - Transferring: {name}")
    body = {"src_site": source_site, "dst_site": target_site,
              "src_path": source_section_path, "dst_path": target_section_path,
              "name": name}
    cbapi.req(f'transferjobs', body=body)

if __name__ == "__main__":
  cbapi.init(cb_api_pass)
  while True:
    mirror()
    time.sleep(check_interval_seconds)
  cbapi.exit(0)
