#!/usr/local/bin/python3

import sys
from scapy.all import *
import codecs
import base64

import warnings
warnings.filterwarnings("ignore")

def parse_hex_base64(payload):
    hext = bytes(payload).hex()
    endc = codecs.decode(hext, 'hex'), 'base64'
    
    text = base64.b64decode(endc[0]).decode('utf-8')

    print(text)

def parse_pcap(pcap_file):

    for packet in PcapReader(pcap_file):
        try:
            if packet[TCP].sport == 10001:
                # print(type(packet[TCP]))  # scapy.layers.inet.TCP
                
                if packet[IP].ttl >= 150:
                    print(f'TTL = {packet[IP].ttl}, TOS = {packet[IP].tos}')
                    print(packet[TCP].mysummary())

                    try:
                        parse_hex_base64(packet[TCP].payload)
                    except Exception as e:
                        print(f"Error on {e}")

                    print("", end = "\n\n")
        except:
            pass

    return 


def main(arguments):
    parse_pcap(arguments[1])


if __name__ == "__main__":
    main(sys.argv)
