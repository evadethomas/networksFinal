# Compression Detector

This project detects if network compression is present on a network path, and is a client/server application. To see the captured packets, please
reference the pcap files.

## This project is broken down into three main phases:

### Pre-probing TCP Phase:
    
    For the first phase, the client sends over a configuration file containing information concerning IP addresses, ports, and packet specifications
    over a TCP connection initialized by the client.

### UDP Probing Phase:

    In the second phase, the client sends over two trains of UDP packets. One with low-entropy data and one with high-entropy data. The server records arrival time
    of these packets and uses the configuration file to determine whether compression is present or not within the packet trains. Note that the client waits an intermediate
    time before sending the second packet.

### Post-Probing TCP Phase:

    Lastly, once the server determines whether or not compression is present, the server returns it's report back to the client via another TCP connection.
    The client then prints the findings of the server.

## Table of contents

- Requirements
- Installation
- Configuration
- Troubleshooting
- FAQ
- Maintainers/Developers
- Citations


## Requirements (required)

This module requires the following module:

    "libcjson" - See installation section for more information on how to use this

## Installation

    To run this project, please download a UTM (Ubuntu) and create a client and a server. For more information on how to do this, see this link:
    "https://docs.google.com/document/d/1ptEYs2jUfzgJ8Ojlpdex4RWdXppA8GJ050ht7pk3z9I/edit"
    
    Once you have two machines running, you need to download libcjson in order to parse the config file. Please run the following:
    
    First, run "sudo apt update"
    Then,  run "sudo apt install libcjson - dev"

    This enables the JSON parser for the JSON file.

    In each machine, I recommend creating a serverFiles folder (on the server), and a clientFiles folder (on the client), as it makes it easier to
    keep track of each file and ensure they are communicating properly. This is optional, otherwise download the following files onto the corresponding machine:

        Client:
            compdetect_client.c
            my_config.json
            cJSON.h
        
        Server:
            compdetect_server.c
            cJSON.h

## Configuration

    Once everything is properly installed, cd to the folders your files are in if neccessary on each machine. Each machine should be running
    at the same time.

    From here, on the client compile the program:
        gcc compdetect_client.c -o compdetect_client -lcjson
    Then compile the server program:
        gcc compdetect_server.c -o compdetect_server -lcjson

    Once both have been compiled and there is certainly an executable on each machine:
    
    First, run the server with the port 7777 by running the following:
        ./compdetect_server 7777
    Then, run the client with the config file:
        ./compdetect_client myconfig.json

    Please wait around 45 seconds for the program to complete.

## Troubleshooting
    
    Make sure to run the machines in the correct order!

    Ensure that you have properly installed the json parser. The cJSON.h and libcjson installation was effective on the creator's virtual
    machines after running these commands. A cJSON.c file should not be needed to run this project.


## Maintainers/Developers

    - Eva DeThomas - egdethomas@dons.usfca.edu
    - Dave Gamble (for the cJSON.h file)

## Citations
     Resources That Aided the Creation and Trouble Shooting of this Project:

        "https://www.geeksforgeeks.org/tcp-server-client-implementation-in-c/"
        "https://www.geeksforgeeks.org/udp-client-server-using-connect-c-implementation/"
        "https://chatgpt.com/"

        JSON.h File: Dave Gamble (2009)
        see: "https://github.com/DaveGamble/cJSON"

## Incomplete features of my program:

        For this project I only implemented Part 1. See each point on the rubric highlighted in the comments within the code. (All features were
        implemented to the best of my knoledge)

	Also please note in the event that a packet is lost, there is a three second time out. This could mean that lost packets may result in inconsistent/correct results.

        I have included some of the beggining stages of part 2, but to see specifically which parts, see compdetect.c file.

