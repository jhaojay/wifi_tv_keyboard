import time
import board
import usb_hid
from adafruit_hid.keyboard import Keyboard
from adafruit_hid.keycode import Keycode

import string

# Generate keycodes for lowercase and uppercase letters
letters_keycodes = {char: getattr(Keycode, char.upper()) for char in string.ascii_letters}

# Generate keycodes for numbers
numbers_keycodes = {char: getattr(Keycode, char) for char in string.digits}

# Combine the dictionaries and add entries for space and backspace
char_to_keycode = {**letters_keycodes, **numbers_keycodes, ' ': Keycode.SPACE, '\b': Keycode.BACKSPACE}


# Initialize the keyboard interface
keyboard = Keyboard(usb_hid.devices)

time.sleep(2)  # Sleep for a bit to avoid sending keys during reset
char = 'A'

keycode = char_to_keycode.get(char)
    if keycode:
        keyboard.press(keycode)
        keyboard.release_all()
        time.sleep(0.1)  # Delay to ensure the keystroke is seen