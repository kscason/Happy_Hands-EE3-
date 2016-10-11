#!/usr/bin/python

"""
learn.py

Program uses supervised machine learning to read in data and store it as a 
  letter of the alphabet based on prompting by the user.
"""

from collections import defaultdict # So we don't have to worry about checking 
      # if a key exists or catching a KeyError exception
from string import ascii_lowercase # Used to check the "count" of each alphabet
      # letter
import cPickle as pickle # cPickle is ~1000 times faster than pickle
  # CAUTION: never unpickle data received from an 
      # untrusted or unauthenticated source!
import os # Low-level stuff. Checking if files exist
import sys # print without newline
from time import sleep
import serial
import threading

VECTOR_LENGTH = 107
# Make threaded vector globally accessible
dynamic_vector = ""

def read_from_port(ser, stop_event):
  global dynamic_vector
  while(not stop_event.is_set()):
    try:
      dynamic_vector = ser.readline().strip() # Constantly update vector (and remove whitespace)
    except KeyboardInterrupt:
      ser.close()


def load_hash_from_pickle(pkl_filename):
  """
  Takes a pickle file and returns the hash table used to run supervised machine
  learning.
  De-serializes the byte stream into a hash table object.

  :param pkl_filename: the file name used to store the pickle
  :return alphabet_hash: the hash table containing the known vectors
  """
  bak_filename = pkl_filename + ".bak"

  # Read in current table from pickle into memory (if it exists)
  if os.path.exists(pkl_filename):
    print("Loading hash table from " + pkl_filename + "...")
    # Load in hash table from pickle
    alphabet_hash = pickle.load(open(pkl_filename, "rb"))
    return alphabet_hash

  # If none exists, look for backup pickle
  if os.path.exists(bak_filename):
    print("Current hash table not found. " +
          "Loading hash table from " + bak_filename + "...")
    # Load in hash table from pickle
    alphabet_hash = pickle.load(open(bak_filename, "rb"))
    return alphabet_hash
  
  # Otherwise, there is no table; start from scratch
  print("Table not found. Creating new hash table...")
  # Table entries saved as dicts
  alphabet_hash = defaultdict(str)
  return alphabet_hash


def dump_hash_to_pickle(pkl_filename, alphabet_hash):
  """
  Serializes the hash table object into a byte stream.

  :param pkl_filename: the file name used to store the pickle
  :param alphabet_hash: the hash table containing the known vectors
  :return:
  """
  bak_filename = pkl_filename + ".bak"
  if os.path.exists(pkl_filename):
    # If current table exists, move table to backup (rename)
    os.rename(os.getcwd() + "/" + pkl_filename, os.getcwd() + "/" + bak_filename)
  # Dump new pickle to filename
  pickle.dump(alphabet_hash, open(pkl_filename, "wb"))


def learn_alphabet(pkl_filename, alphabet_hash):
  """
  Takes a hash table (defaultdict) and updates it with newly learned vectors 
  based on user input. Special behavior for gestures. Arduino pre-processes the 
  accelerometer data to identify gesture-based movement and hand orientation.
  Loops indefinitely until user prompts "quit".

  :param pkl_filename: the file name used to store the pickle
  :param alphabet_hash: the hash table containing the known vectors
  :return:
  """

  # Infinitely loop while user is "teaching" letters
  while(True):
    # Prompt user for alphabet letter (or "quit")
    user_input = raw_input("Enter the letter that you are signing (or \'quit\', 'clean\', \'save\' or\n\'count\'): ").lower()

    if (user_input == "quit"):
      print("Quitting...")
      return
    elif (user_input == "clean"):
      # Clean dictionary (remove empty entries and entries in neutral position
      for key in alphabet_hash.keys():
        if not alphabet_hash[key] or key[1] == 'N':
          del alphabet_hash[key]
      print("Pickle cleaned!")
    elif (user_input == "save"):
      dump_hash_to_pickle(pkl_filename, alphabet_hash)
      print("Pickle saved!")
    elif (user_input == "count"):
      letter_count = defaultdict(int)
      for val in alphabet_hash.values():
        if val:
          letter_count[val] += 1
      print("Letter count:")
      print(", ".join([letter + ": " + str(letter_count[letter])
        for letter in ascii_lowercase]))
    elif (user_input >= "a" and user_input <= "z"):

      vector = dynamic_vector # Take snapshot of vector

      # Vector not in hash table
      if (not vector):
        print("Resource busy or not available. Try again.")
      elif (len(vector) != VECTOR_LENGTH):
        print("Vector length of " + str(len(vector)) + " is incorrect.")
      elif (not alphabet_hash[vector]):
        # Create new hash table entry
        pretty_print(vector)
        while(True):
          conditional_input = raw_input("mapping to letter \'" + user_input + 
              "\'. Store mapping? [y/n] ").lower()
          if (not conditional_input):
            pass
          elif (conditional_input[0] == "y"):
            alphabet_hash[vector] = user_input
            print("Vector mapped to letter \'" + user_input + "\'")
            break
          elif (conditional_input[0] == "n"):
            print("Vector not mapped.")
            break
      # Vector is associated with a different alphabet letter
      elif (alphabet_hash[vector] != user_input):
        print("MAPPING CONFLICT!")
        pretty_print(vector)
        print("is mapped to letter \'" + alphabet_hash[vector] + 
            "\' but user requesting mapping to letter \'" + user_input + "\'.")
        while(True):
          conditional_input = raw_input("Map to letter \'" + user_input + 
              "\' instead? [y/n] ").lower()
          if(not conditional_input):
            pass
          elif (conditional_input[0] == "y"):
            alphabet_hash[vector] = user_input
            pretty_print(vector)
            print("mapped to letter \'" + user_input + "\'")
            break
          elif (conditional_input[0] == "n"):
            print("Mapping unchanged.")
            break

      # Just print the vector and alphabet letter
      else:
        pretty_print(vector)
        print("Already mapped to letter \'" + user_input + "\'")

      #
      # Special cases: Z, J, tap/glide? (gesture-based)
      #

    else:
      print("Input invalid. Please try again.")


def pretty_print(vector):
  """
  Parses the input vector and displays it in human readable format

  :param vector: Data passed in from glove that defines one ASL alphabet letter
    ~Format~
    vector[0] ::= Movement
    vector[1] ::= Orientation
    vector[2:6] ::= Flex sensors 1 - 5
    vector[7:] ::= Contacts (full matrix?)
  :return:
  """
  # Movement
  if (vector[0] == 'S'):
    print("Movement: Static")
  elif (vector[0] == 'M'):
    print("Movement: Moving")

  # Orientation
  if (vector[1] == 'U'):
    print("Orientation: Upright")
  elif (vector[1] == 'S'):
    print("Orientation: Sideways")
  elif (vector[1] == 'D'):
    print("Orientation: Downwards")
  elif (vector[1] == 'N'):
    print("Orientation: Neutral")
  # Not sure about j, z

  # Flex bend
  # Thumb
  if (vector[2] == 'S'):
    print("Thumb: Straight")
  elif (vector[2] == 'N'):
    print("Thumb: Not Straight")

  # Index
  if (vector[3] == 'S'):
    print("Index: Straight")
  if (vector[3] == 'C'):
    print("Index: Curled")
  elif (vector[3] == 'B'):
    print("Index: Bent")

  # Middle
  if (vector[4] == 'S'):
    print("Middle: Straight")
  elif (vector[4] == 'N'):
    print("Middle: Not Straight")

  # Ring
  if (vector[5] == 'S'):
    print("Ring: Straight")
  elif (vector[5] == 'N'):
    print("Ring: Not Straight")

  # Pinkie
  if (vector[6] == 'S'):
    print("Pinkie: Straight")
  elif (vector[6] == 'N'):
    print("Pinkie: Not Straight")

  sys.stdout.write("\t");
  for i in range(10):
    sys.stdout.write(str(i + 1) + "  ");

  for i in range(100):
    if (i % 10 == 0):
      print("")
      sys.stdout.write(str(i/10 + 1) + "\t")
    sys.stdout.write(str(vector[7 + i]) + "  ")
  print("")

if __name__ == "__main__":
  # Use relative path
  pkl_filename = "alphabet_hash.pkl"  # default table file

  port_name = "/dev/tty.Mochi-SPP" # Bluetooth connection
  #port_name = "/dev/tty.usbmodem1172121" # Serial connection for Teensy LC
  serial_speed = 115200

  # Check if serial port exists
  if (not os.path.exists(port_name)):
    sys.exit("Could not find serial port.")

  print("Connecting to serial port...")
  # Initialize serial port at 115200 baud rate, 8N1 default
  try:
    ser = serial.Serial(port_name, serial_speed, timeout=1)
  except OSError:
    sys.exit("Resource busy or not available. Try again.")

  # Read from port in a different thread
  thread_stop = threading.Event()
  thread = threading.Thread(target=read_from_port, args=(ser, thread_stop))
  thread.start()

  # Load hash table from pickle
  alphabet_hash = load_hash_from_pickle(pkl_filename)
  print("Hash table successful!")

  # Run supervised machine learning algorithm
  try:
    learn_alphabet(pkl_filename, alphabet_hash)
  except KeyboardInterrupt:
    ser.close()
    thread_stop.set()
    sys.exit("Exit on user SIGINT. Port closed.")

  thread_stop.set()

  # Dump hash table to pickle
  dump_hash_to_pickle(pkl_filename, alphabet_hash)

  print("New table successfully saved! Exiting program.")

