from Crypto.Cipher import AES

# --------------------------
# GIVEN ORACLE (from lab)
# --------------------------
KEY = b"THIS_IS_A_SECRET"      # Not known to attacker
IV  = b"RANDOM_INITVECTR"      # Must be 16 bytes for AES CBC

def decrypt_and_check(ciphertext: bytes) -> bool:
    """Returns True if PKCS#7 padding is valid, False otherwise."""
    cipher = AES.new(KEY, AES.MODE_CBC, IV)
    plaintext = cipher.decrypt(ciphertext)

    pad = plaintext[-1]
    if pad < 1 or pad > 16:
        return False
    if plaintext[-pad:] != bytes([pad]) * pad:
        return False
    return True


# --------------------------
# STEP 1 — is_valid()
# --------------------------
def is_valid(ciphertext: bytes):
    if decrypt_and_check(ciphertext):
        print("Valid Padding")
        return True
    else:
        print("Invalid Padding")
        return False


# --------------------------
# TARGET BLOCK (YOUR CIPHERTEXT)
# --------------------------
target_hex = "b4e37a7b64198687e5aac41d9ac98da5"
target_block = bytes.fromhex(target_hex)


# --------------------------
# STEP 2 — Single-byte brute force on last byte of IV
# --------------------------
def brute_force_last_byte():
    print("\n[*] Brute forcing last byte of previous block...\n")

    original_prev = bytearray(IV)
    found = []

    for guess in range(256):
        mod_prev = bytearray(original_prev)
        mod_prev[-1] = guess       

        ciphertext = bytes(mod_prev) + target_block

        if decrypt_and_check(ciphertext):
            print(f"[+] Valid padding for guess: {guess}")
            found.append(guess)

    print("\nTotal valid guesses:", found)
    return found


# --------------------------
# STEP 3 — Full padding-oracle attack (all 16 bytes)
# --------------------------
def recover_block(cipher_block: bytes, prev_block: bytes) -> bytes:
    recovered = [0] * 16
    intermediate = [0] * 16
    orig_prev = bytearray(prev_block)

    for pos in range(15, -1, -1):
        pad = 16 - pos
        print(f"\n[***] Recovering byte {pos} (pad={pad})")

        mod_prev = bytearray(orig_prev)

        # Fix already recovered bytes to match desired padding
        for j in range(15, pos, -1):
            mod_prev[j] = intermediate[j] ^ pad

        # Try all 256 possibilities for current byte
        for guess in range(256):
            mod_prev[pos] = guess
            ciphertext = bytes(mod_prev) + cipher_block

            if decrypt_and_check(ciphertext):
                intermediate[pos] = guess ^ pad
                recovered[pos] = intermediate[pos] ^ orig_prev[pos]

                print(f"[+] Found byte: 0x{recovered[pos]:02x}")
                break

    return bytes(recovered)


# --------------------------
# EXECUTION
# --------------------------
print("\n== Checking original block ==")
is_valid(target_block)

print("\n== Step 2: single-byte brute force ==")
brute_force_last_byte()

print("\n== Step 3: full block recovery ==")
recovered = recover_block(target_block, IV)

print("\nRecovered bytes (hex):")
print(" ".join(f"{b:02x}" for b in recovered))

print("\nASCII:")
print("".join(chr(b) if 32 <= b <= 126 else '.' for b in recovered))
