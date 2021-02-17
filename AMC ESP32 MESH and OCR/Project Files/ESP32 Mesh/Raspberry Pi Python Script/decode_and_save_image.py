#!/usr/bin/env python
import re
import base64

#This is our regex created to filter out the base64

pattern = r"msg\"\:\"([^&]+?)\"" #taken from https://regex101.com/r/lHxPKP/1/codegen?language=python

file = open("/home/pi/broadcast.txt", "r")

for line in file:

    match = re.search(pattern, line)

    if match:
    #Use match.group(1) as that is the variable we want to extract to decode
        base64var = match.group(1)
        base64var_bytes = base64var.encode('utf-8')
        with open('decoded_image.jpg', 'wb') as file_to_save:
            decoded_image_data = base64.decodebytes(base64var_bytes)
            file_to_save.write(decoded_image_data)

    else:
        print("Error. No base64 detected.")