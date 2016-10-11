#!/usr/bin/python

"""
display.py

Program uses byte stream of defaultdict from supervised machine learning to 
  determine the letter of the alphabet the user is signing. Uses Bluetooth
  SPP (serial port profile) connection.
"""

COUNT_THRESHOLD = 125

import learn # Use previous code to load
from collections import defaultdict # So we don't have to worry about checking 
      # if a key exists or catching a KeyError exception
from string import ascii_lowercase
import cPickle as pickle # cPickle is ~1000 times faster than pickle
  # CAUTION: never unpickle data received from an 
      # untrusted or unauthenticated source!
import os # Low-level stuff. Checking if files exist
import sys # print without newline
from time import sleep
import serial
import threading

def display_alphabet(alphabet_hash):
  vector = str
  previous_letter = str
  count = 0
  # Loop indefinitely, reading in user fingerspelling
  while (True):
    vector = learn.dynamic_vector # Take snapshot of vector

    # No data received; serial port timed out
    if (not vector):
      print("\033c")
      print("Attempting to read data...")
    elif (len(vector) != learn.VECTOR_LENGTH):
      print("\033c")
      print("Vector length of " + str(len(vector)) + " is incorrect.")
    else:
      # print(chr(27) + "[2J")
      if alphabet_hash[vector] and alphabet_hash[vector] != previous_letter:
        count = 0
        previous_letter = alphabet_hash[vector]
      elif alphabet_hash[vector] == previous_letter:
        count += 1
    if alphabet_hash[vector] and count == COUNT_THRESHOLD:
      print("\033c") # Flush screen
      print(alphabet_hash[vector])


if __name__ == "__main__":
  # Use relative path
  pkl_filename = "alphabet_hash.pkl"  # default table file

  #port_name = "/dev/tty.usbmodem1172121" # Serial connection for Teensy LC
  port_name = "/dev/tty.Mochi-SPP"
  serial_speed = 115200

  # Check if serial port exists
  if (not os.path.exists(port_name)):
    sys.exit("Could not find serial port.")

  print("Connecting to serial port...")
  # Setup serial port
  try:
    # Timeout required for function readline()
    ser = serial.Serial(port_name, serial_speed, timeout=1)
  except OSError:
    sys.exit("Resource busy or not available. Try again.")

  # Read from port in a different thread
  thread_stop = threading.Event()
  thread = threading.Thread(target=learn.read_from_port, args=(ser, thread_stop))
  thread.start()

  # Load hash table from pickle
  alphabet_hash = learn.load_hash_from_pickle(pkl_filename)
  print("Hash table successful!")

  # Run display fingerspelling method
  try:
    display_alphabet(alphabet_hash)
  # Simplest way to catch SIGINT
  except KeyboardInterrupt:
    print("")
    ser.close()
    thread_stop.set()
    sys.exit("Exit on user SIGINT. Port closed.")
