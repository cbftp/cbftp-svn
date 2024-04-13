#!/usr/bin/env python3
#
# This is a simple api script that resets all transfer jobs.
#
import cbapi

cb_api_pass = 'your_api_password'

cbapi.init(cb_api_pass)
transferjobs = cbapi.req("transferjobs?id=true")
for transferjob in transferjobs:
  id = transferjob["id"]
  cbapi.req(f"transferjobs/{id}/reset?id=true", '{}')
cbapi.exit(0)
