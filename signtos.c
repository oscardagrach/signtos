#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <openssl/cmac.h>

/* written by oscardagrach */

#define SIG_LEN 16
#define HDR_LEN	0x40
#define HDR_IMG_LEN_OFFSET 0x7

/*
 * if you use anything other than
 * a zeroed sbk (for development/unfused devices) 
 * you'll need to declare that somewhere before
 * the image is signed
 */
static unsigned char key[SIG_LEN] = {0};

int main(int argc, char *argv[])
{
	int err, i, size = 0;
	FILE *inf;
	char file[200] = {0};
	size_t siglen;
	unsigned char sig[SIG_LEN] = {0};
	unsigned char hdr[HDR_LEN] = {0};
	unsigned char *message;
	unsigned char *output;

	if (argc > 2) {
		printf("Too many arguments\n");
		return -EINVAL;
	} else if (!argv[1]) {
		printf("No image provided\n");
		return -EINVAL;
	} else {
		printf("Reading image...\n\n");
		sscanf(argv[1], "%200s", file);
	}

	/* open file */
	inf = fopen(file, "rb");
	if (!inf) {
		printf("Failed to open file\n");
		err = -ENODEV;
		goto out;
	}

	/* read header to buffer */
	err = fread(hdr, 1, HDR_LEN, inf);
	if (err != HDR_LEN) {
		printf("Failed to read header into buffer\n");
		err = -ENOMEM;
		goto out;
	}

	/* move file pointer back to beginning of file */
	rewind(inf);

	/* read image size from header */
	sscanf((hdr + HDR_IMG_LEN_OFFSET), "%d", &size);
	size += 512; /* add the size of the header to the image size */

	printf("Expected image size: 0x%X\n\n", size);

	/* data to be signed */
	message = malloc(sizeof(unsigned char)*(size));
	if (!message) {
		printf("Failed to allocate image buffer\n");
		err = -ENOMEM;
		goto out;
	}

	/* data + sig to be written to file */
	output = malloc(sizeof(unsigned char)*(size + SIG_LEN));
	if (!output) {
		printf("Failed to allocate output buffer\n");
		err = -ENOMEM;
		goto out;
	}

	/* read image to buffer */
	err = fread(message, 1, size, inf);;
	if (err != size) {
		printf("Failed to read file into buffer\n");
		err = -ENOMEM;
		goto out;
	}

	fclose(inf);

	/* initialize CMAC context */
	CMAC_CTX *ctx = CMAC_CTX_new();

	printf("SBK: ");
	for (i=0; i<SIG_LEN; i++) {
		printf("%02hhX", key[i]);
	}
	printf("\n");

	/* initialize CMAC key and cipher */
	CMAC_Init(ctx, key, 16, EVP_aes_128_cbc(), NULL);

	/* update CMAC context */
	CMAC_Update(ctx, message, size);

	/* finalize CMAC */
	CMAC_Final(ctx, sig, &siglen);

	/* free up context */
	CMAC_CTX_free(ctx);

	printf("Signature: ");
	for (i=0; i<SIG_LEN; i++) {
		printf("%02hhX", sig[i]);
	}
	printf("\n\n");

	/* copy image that was signed to buffer to append signature */
	memcpy(output, message, size);
	/* append signature to image output */
	memcpy((output+size), sig, SIG_LEN);

	inf = fopen("tos_signed.bin", "wb");
	if (!inf) {
		printf("Failed to open file for writing\n");
		err = -EINVAL;
		goto out;
	}

	if (!fwrite(output, 1, (size + SIG_LEN), inf)) {
		printf("Failed to write file\n");
		err = -EINVAL;
		goto out;
	}

	err = 0;
	printf("Signed TOS image successfully\n");

out:
	if (message)
		free(message);
	if (output)
		free(output);
	if (inf) {
		fsync(fileno(inf));
		fclose(inf);
	}

	return err;
}
