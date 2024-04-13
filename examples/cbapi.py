#!/usr/bin/env python3

# this module contains helper functions for using the cbftp api. Example usage:
#
#  import cbapi
#  cbapi.init('your_api_password')
#  info = cbapi.req('info')
#  print(f"openssl version is: {info['build_info']['openssl_version']}")
#  cbapi.exit(0)
#

cb_api_port = 55477
cb_api_pass = 'default'

import json
import ssl
import sys
import urllib3

urllib3.disable_warnings(urllib3.exceptions.InsecureRequestWarning)
pool_mgr = urllib3.PoolManager(cert_reqs=ssl.CERT_NONE)

def _shutdown_tls_sessions_nicely():
  # This is not really needed but makes cbftp consider the api connection
  # gracefully closed.
  for pool_key in pool_mgr.pools.keys():
    pool = pool_mgr.pools[pool_key]
    for https_conn in pool.pool.queue:
      if https_conn:
        https_conn.sock.unwrap()

def init(password, port=55477):
  global cb_api_port
  global cb_api_pass
  cb_api_port = port
  cb_api_pass = password

def exit(status):
  _shutdown_tls_sessions_nicely()
  sys.exit(status)

def req(path, body=None):
  headers = urllib3.make_headers(basic_auth=':%s' % cb_api_pass)
  command = 'GET' if body is None else 'POST'
  r = pool_mgr.request(command, 'https://localhost:%s/%s' % (cb_api_port, path),
    headers=headers, body=None if body is None else json.dumps(body))
  if r.status >= 400:
    if r.status == 401:
      print('Error: invalid API token')
    elif r.status == 404:
      print(f"Error: path not found: {path}")
    else:
      print(f"Error: Received HTTP status {r.status} for {path}")
    exit(1)
  if len(r.data) == 0:
    return {}
  return json.loads(r.data)
