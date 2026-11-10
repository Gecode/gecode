"""Deadline-bound transport for the two interactive Word SMT adapters."""

from contextlib import contextmanager
import os
import selectors
import subprocess
import time
from typing import Iterator

CLEANUP_SECONDS = 1.0


class SmtProcess:
  """Exchange complete SMT response lines under one monotonic deadline."""

  def __init__(self, process: subprocess.Popen, selector: selectors.BaseSelector,
               deadline: float, timeout: float):
    self.process = process
    self.selector = selector
    self.deadline = deadline
    self.timeout = timeout
    self.pending = bytearray()
    self.has_eof = False

  def write(self, text: str) -> None:
    data = memoryview(text.encode("utf-8"))
    self.selector.register(self.process.stdin, selectors.EVENT_WRITE)
    try:
      while data:
        # Read concurrently: a solver may reply before consuming all input.
        for key, _ in self._wait():
          if key.fileobj is self.process.stdout:
            self._read_chunk()
          else:
            try:
              sent = os.write(key.fd, data[:65536])
              data = data[sent:]
            except BlockingIOError:
              pass
        if self.has_eof and data:
          raise RuntimeError("solver closed its output while input was pending")
    finally:
      self.selector.unregister(self.process.stdin)

  def read_response(self) -> str:
    text = self._read_line().strip()
    if text.startswith("("):
      balance = text.count("(") - text.count(")")
      while balance > 0:
        part = self._read_line().strip()
        text += " " + part
        balance += part.count("(") - part.count(")")
    return text

  def _get_remaining(self) -> float:
    remaining = self.deadline - time.monotonic()
    if remaining <= 0:
      raise subprocess.TimeoutExpired(self.process.args, self.timeout)
    return remaining

  def _wait(self) -> list:
    events = self.selector.select(self._get_remaining())
    if not events:
      raise subprocess.TimeoutExpired(self.process.args, self.timeout)
    return events

  def _read_chunk(self) -> None:
    try:
      chunk = os.read(self.process.stdout.fileno(), 65536)
    except BlockingIOError:
      return
    if chunk:
      self.pending.extend(chunk)
    else:
      self.has_eof = True
      self.selector.unregister(self.process.stdout)

  def _read_line(self) -> str:
    while True:
      self._get_remaining()
      end = self.pending.find(b"\n")
      if end >= 0:
        line = bytes(self.pending[:end])
        del self.pending[:end + 1]
        return line.decode("utf-8", errors="replace")
      if self.has_eof:
        if self.pending:
          line = bytes(self.pending)
          self.pending.clear()
          return line.decode("utf-8", errors="replace")
        raise RuntimeError("solver closed its output before a complete response")
      self._wait()
      self._read_chunk()


@contextmanager
def open_solver(command: list[str], deadline: float,
                timeout: float) -> Iterator[SmtProcess]:
  """Own solver pipes and reap the child with at most one second of cleanup."""
  # Diagnostics were unused by both adapters; discarding them avoids pipe stalls.
  process = subprocess.Popen(command, stdin=subprocess.PIPE,
                             stdout=subprocess.PIPE, stderr=subprocess.DEVNULL,
                             bufsize=0)
  try:
    with selectors.DefaultSelector() as selector:
      os.set_blocking(process.stdin.fileno(), False)
      os.set_blocking(process.stdout.fileno(), False)
      selector.register(process.stdout, selectors.EVENT_READ)
      yield SmtProcess(process, selector, deadline, timeout)
  finally:
    # Unbuffered streams cannot flush pending writes while closing.
    process.stdin.close()
    process.stdout.close()
    process.kill()
    process.wait(timeout=CLEANUP_SECONDS)
