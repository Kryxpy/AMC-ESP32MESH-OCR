from pytesseract import*
import numpy as np
import argparse
import cv2


# This section constructs the argument parser for --image and parses the arguments
ap = argparse.ArgumentParser()

ap.add_argument("-i", "--image",
                help="path to input image to be OCR'd")
ap.add_argument("-c", "--min-conf",
                type=int, default=0,
                help="mininum confidence value to filter weak text detection")
args = vars(ap.parse_args())

#This Section specifies the cropping area of the image
images = cv2.imread(args["image"])
y=360
x=250
h=40
w=400
crop = images[y:y+h, x:x+w]

#Load the input image and convert from RGB to BGR
#Then, we can use Pytesserect to localise each area of text in the image
rgb = cv2.cvtColor(crop, cv2.COLOR_BGR2RGB)
results = pytesseract.image_to_data(rgb, output_type=Output.DICT)

# This Section loops over all the individual text found within the image and prints the result
for i in range(0, len(results["text"])):
    # We can then extract the bounding box coordinates
    x = results["left"][i]
    y = results["top"][i]
    w = results["width"][i]
    h = results["height"][i]

    # Extracts the OCR text and the confidence of text localization
    text = results["text"][i]
    conf = int(results["conf"][i])

    # filter out low confidence text localizations and display the result in the terminal
    if conf > args["min_conf"]:
        if conf > 1 :
         print("Confidence: {}".format(conf))
         print("Text: {}".format(text))
         print("")

        # Strip out non-ASCII text
        text = "".join(text).strip()
        cv2.rectangle(images,
                      (x, y),
                      (x + w, y + h),
                      (0, 0, 255), 2)
        cv2.putText(images,
                    text,
                    (x, y - 10),
                    cv2.FONT_HERSHEY_SIMPLEX,
                    1.2, (0, 255, 255), 3)

# Display the output image
cv2.imshow("Image", crop)
cv2.waitKey(0)
quit(0)