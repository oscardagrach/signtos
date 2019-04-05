# signtos
Sign TOS images for Nvidia Tegra SoCs

This program signs a TOS (TrustZone/Trusted OS) image for the Nvidia Tegra SoCs. All that
is needed is to supply a TOS image as an argument. Any trailing garbage/data will be truncated
and the image signed, verified by the code length in the header. Nvidia uses AES-CMAC to sign the
image using the secure boot key in fuses, or a zeroed key in the case of development/unfused devices.

Tested on T114/T124/T210

#### Dependencies:
libssl-dev

#### Building:
```
make
```

#### Usage:
```
./signtos tos.bin
```

```
oscardagrach@ubuntu:$ ./signtos tos.bin 
Reading image...

Expected image size: 0xD3B0

SBK: 00000000000000000000000000000000
Signature: E35D485854CF2D450EDA1D50C142B908

Signed TOS image successfully
```
