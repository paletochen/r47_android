#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
# SPDX-FileCopyrightText: Copyright The C47 Authors
#
# Monitor of the HP 82240 byte stream that the C47 and R47 simulators send over UDP.
#
# sendByteIR() in src/c47-gtk/hal/print_ir.c sends every printer byte as one datagram to 127.0.0.1:5025, the port of the HP82240B simulator by Christoph Giesselink
# and of the WP 34s Qt printer emulator. This script receives the same datagrams and shows each printed line with its timing, so a print can be checked on any host
# without a printer simulator. Only the Python standard library is used, so it runs unchanged on Windows, macOS and Linux.
#
# Only one program can receive on a port. Stop the printer simulator first, or move it to another port and hand the bytes on with --forward.
#
# Usage:
#   python3 tools/printer/printer-udp-monitor.py                          show the printed lines, Ctrl+C ends and prints the summary
#   python3 tools/printer/printer-udp-monitor.py --hex                    show every byte in hexadecimal as well
#   python3 tools/printer/printer-udp-monitor.py --save print.bin         write the received bytes to a file
#   python3 tools/printer/printer-udp-monitor.py --forward 127.0.0.1:5026 hand every byte on to a printer simulator on port 5026
#   python3 tools/printer/printer-udp-monitor.py --timeout 5              end 5 s after the last byte
#   python3 tools/printer/printer-udp-monitor.py --replay print.bin       send a saved capture to the port, one byte per datagram
#   python3 tools/printer/printer-udp-monitor.py --replay print.bin --rate 500   the same at 500 bytes per second
#
# The summary reports the byte count, the time from the first to the last byte, the rate and the longest gap between two bytes. A long gap in the middle of a print
# is the measurement to report with a simulator that stops responding while it prints.

import argparse
import socket
import sys
import time

ESC = 27
LINE_FEEDS = (0x04, 0x0A)
PAPER_COLUMNS = 166


class Decoder:
  """Splits the HP 82240 byte stream into printed lines. Graphic data is shown as [G n] for n dot columns, other bytes outside ASCII as <XX>."""

  def __init__(self):
    self.escape = False
    self.graphic = 0
    self.line = ''

  def feed(self, byte):
    """Takes one byte and returns the finished line on a line feed, otherwise None."""
    if self.escape:
      self.escape = False
      if 1 <= byte <= PAPER_COLUMNS:
        self.graphic = byte
        self.line += '[G %d]' % byte
      else:
        self.line += '[ESC %d]' % byte
      return None
    if self.graphic > 0:
      self.graphic -= 1
      return None
    if byte == ESC:
      self.escape = True
      return None
    if byte in LINE_FEEDS:
      line, self.line = self.line, ''
      return line
    self.line += chr(byte) if 32 <= byte < 127 else '<%02X>' % byte
    return None


def monitor(args):
  sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
  try:
    sock.bind((args.host, args.port))
  except OSError as error:
    sys.exit('cannot receive on %s:%d (%s). A printer simulator on that port has to be stopped first.' % (args.host, args.port, error))
  sock.settimeout(0.2)  # a short timeout lets Ctrl+C end the script on Windows too

  forward = None
  if args.forward:
    host, port = args.forward.rsplit(':', 1)
    forward = (host, int(port))
  save = open(args.save, 'wb') if args.save else None

  decoder = Decoder()
  count = 0
  first = last = lastLine = None
  longestGap = 0.0
  print('receiving on %s:%d, Ctrl+C ends' % (args.host, args.port), flush=True)
  try:
    while True:
      try:
        data, _ = sock.recvfrom(4096)
      except socket.timeout:
        if args.timeout and last is not None and time.monotonic() - last > args.timeout:
          break
        continue
      now = time.monotonic()
      if first is None:
        first = lastLine = now
      elif now - last > longestGap:
        longestGap = now - last
      last = now
      if forward:
        sock.sendto(data, forward)
      if save:
        save.write(data)
      for byte in data:
        count += 1
        if args.hex:
          print('%8.3f  %02X' % (now - first, byte))
        line = decoder.feed(byte)
        if line is not None:
          print('%8.3f %+7.3f  %s' % (now - first, now - lastLine, line), flush=True)
          lastLine = now
  except KeyboardInterrupt:
    pass
  finally:
    if save:
      save.close()
    sock.close()

  if decoder.line:
    print('         (line without line feed)  %s' % decoder.line)
  if count == 0:
    print('no bytes received')
    return
  span = last - first
  print('%d bytes in %.3f s, %.0f bytes/s, longest gap between two bytes %.3f s' % (count, span, count / span if span > 0 else 0, longestGap))


def replay(args):
  with open(args.replay, 'rb') as capture:
    data = capture.read()
  sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
  pause = 1.0 / args.rate if args.rate else 0.0
  for byte in data:
    sock.sendto(bytes([byte]), (args.host, args.port))
    if pause:
      time.sleep(pause)
  sock.close()
  print('%d bytes sent to %s:%d' % (len(data), args.host, args.port))


def main():
  parser = argparse.ArgumentParser(description='Monitor or replay the HP 82240 byte stream of the C47 and R47 simulators.')
  parser.add_argument('--host', default='127.0.0.1', help='address to receive on or send to (default 127.0.0.1)')
  parser.add_argument('--port', type=int, default=5025, help='UDP port (default 5025, as in print_ir.c)')
  parser.add_argument('--hex', action='store_true', help='show every byte in hexadecimal')
  parser.add_argument('--save', metavar='FILE', help='write the received bytes to FILE')
  parser.add_argument('--forward', metavar='HOST:PORT', help='hand every received datagram on to HOST:PORT')
  parser.add_argument('--timeout', type=float, metavar='SEC', help='end SEC seconds after the last byte')
  parser.add_argument('--replay', metavar='FILE', help='send the bytes of FILE instead of receiving')
  parser.add_argument('--rate', type=float, metavar='BYTES', help='with --replay, bytes per second (default as fast as possible)')
  args = parser.parse_args()
  if args.replay:
    replay(args)
  else:
    monitor(args)


if __name__ == '__main__':
  main()
