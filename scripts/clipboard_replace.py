import pyperclip
import re
import time

def on_clipboard_change(previous_clipboard):
    current_clipboard = pyperclip.paste()
    if current_clipboard != previous_clipboard:
        modified_clipboard = re.sub(r'.*?(\..*?);', r'\1,', current_clipboard)
        if modified_clipboard != current_clipboard:
            pyperclip.copy(modified_clipboard)
        return modified_clipboard
    return previous_clipboard

def main():
    previous_clipboard = pyperclip.paste()
    while True:
        previous_clipboard = on_clipboard_change(previous_clipboard)
        time.sleep(1)

if __name__ == "__main__":
    main()