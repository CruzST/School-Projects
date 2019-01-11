# One Time Pad

This program utilizes socket programming to simulate a "One Time Pad" encryption technique.

## Steps of a basic application:

* The User will use otp_enc_d.c and otp_dec_d.c to set up a socket to "listen" for an incoming encrypted message.
* The user will generate a key file using keygen.c.
* The user will then use otp_enc.c or otp_dec.c to send the key file and a text file that is either unencrypted or encrypted to the listening servers.
* The programs will then scramble or descramble the code and send it back to the sender.


## Submission includes 12 files

=== program files ===

otp_enc.c

otp_enc_d.c

otp_dec.c

otp_dec_d.c

keygen.c

=== text files ===

plaintext1

plaintext2

plaintext3

plaintext4

plaintext5

=== bash scripts ===

compileall

p4gradingscript

(scripts require chmod +x permissions)
