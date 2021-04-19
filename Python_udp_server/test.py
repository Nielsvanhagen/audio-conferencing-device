import pyaudio
import socket
import struct

UDP_IP = "192.168.178.15"
UDP_PORT = 3333

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((UDP_IP, UDP_PORT))

p = pyaudio.PyAudio()
FORMAT = pyaudio.paInt16
CHANNELS = 1
RATE = 16000
CHUNK = 1024
#stream = p.open(format=pyaudio.paInt16, channels=1, rate=44100, output=True)
stream = p.open(format=FORMAT, channels=CHANNELS, rate=RATE, input=True, output=True, frames_per_buffer=CHUNK)
frames = []

try:
    while True:
        data, addr = sock.recvfrom(CHUNK*2) # buffer de 1024 bytes
        # for i in range(0,len(data)):
        #     print(data[i])
        #stream.write(data)
        # print(data)
        sock.sendto(data, addr)
        
except KeyboardInterrupt:  
    print("Stop streaming")
    stream.stop_stream()
    stream.close()
    p.terminate()