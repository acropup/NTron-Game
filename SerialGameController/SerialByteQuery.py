print("This script connects to a 9600-baud Serial device")
print("and sends '?' as a query character.")
print("It then reads a byte and prints it out in binary format.")

try:
  import serial
except:
  print("Need to install pySerial. Run 'pip install pyserial' on command line.")

ser = 0
portName = '' #Something like 'COM3' on Windows. Check Arduino IDE Tools->Port menu.

def connect():
  global ser, portName
  while True:
    try:
      ser = serial.Serial(portName, timeout=1)
      return
    except:
      portName = input('Enter the serial port name: ')

def query():
  connect()
  while True:
    x = input('Press Enter to query. Any other character to quit.')
    if x != '':
      return
    try:
      ser.write(b'?')
      while True:
        print(bin(ser.read()[0])[2:].zfill(8))
        if ser.in_waiting == 0: break
    except:
      print('Serial failed. Trying to reconnect.')
      connect()

query()
