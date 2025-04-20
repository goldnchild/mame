// license:BSD-3-Clause
// copyright-holders: Original author, Robbbert
/********************************************************************

    Support for Spectravideo Compumate cassette images

    bin files are either 0x604, 0x804 bytes in length,
    0x604 for picturemate and basic programs
    0x804 for songmate

    header consists of approx 10 seconds of 0xff bytes

    each byte is a 0 start bit followed by 8 data bits followed by four 1 stop bits

    0 bit is encoded as 1 cycle of 8 samples @ 11025 (approx 1400 hz * 1 cycle)
    1 bit is encoded as 2 cycles of 5 samples @ 11025 (approx 2200 hz * 2 cycles)

********************************************************************/

#include "formats/compumate_cas.h"

#define WAVEENTRY_LOW  -32768
#define WAVEENTRY_HIGH  32767

/* frequency of wave */
#define COMPUMATE_WAV_FREQUENCY  22050

#define SAMPLES_PER_0 (18)
#define SAMPLES_PER_1 (9)
#define HEADER_TIME_SECS (10)
#define HEADER_LENGTH ((COMPUMATE_WAV_FREQUENCY * HEADER_TIME_SECS) / (SAMPLES_PER_0 + ((8 + 4) * 2) * SAMPLES_PER_1) + 1)


static int compumate_image_size;

static int compumate_put_samples(int16_t *buffer, int sample_pos, int count, int level)
{
	if (buffer)
	{
		for (int i = 0; i < count; i++)
			buffer[sample_pos + i] = level;
	}

	return count;
}

static int compumate_output_bit(int16_t *buffer, int sample_pos, bool bit)
{
	int samples = 0;

	const int samples_per_hc_low[]  = {9, 5};  // half cycle samples low
	const int samples_per_hc_high[] = {9, 4};  // half cycle samples high

	const int cycles_per[] = {1, 2};  // compumate has 1 cycle for a 0 bit, 2 cycles for a 1 bit

	for (int i = 0; i < cycles_per[bit]; i++)
	{
		samples += compumate_put_samples(buffer, sample_pos + samples, samples_per_hc_low[bit], WAVEENTRY_LOW);
		samples += compumate_put_samples(buffer, sample_pos + samples, samples_per_hc_high[bit], WAVEENTRY_HIGH);
	}

	return samples;
}

static int compumate_output_byte(int16_t *buffer, int sample_pos, uint8_t data)
{
	int samples = 0;
	const int start_bits = 1;
	const int data_bits = 8;
	const int stop_bits = 4;

	for (int i = 0; i < start_bits; i++)
		samples += compumate_output_bit (buffer, sample_pos + samples, 0);  // 1 start bit of zero

	for (int i = 0; i < data_bits; i++)
		samples += compumate_output_bit (buffer, sample_pos + samples, ((data >> i) & 1));  // lsb first

	for (int i = 0; i < stop_bits; i++)
		samples += compumate_output_bit (buffer, sample_pos + samples, 1);  // 4 stop bits

	return samples;
}


static int compumate_bin_handle_cassette(int16_t *buffer, const uint8_t *bytes)
{
	uint32_t sample_count = 0;

	// silence
	//sample_count += compumate_put_samples(buffer, 0, 12000, 0);

	/* start */ // roughly 900 bytes of 0xff as header for 10 seconds (can be much much less)
	for (int i = 0; i < HEADER_LENGTH; i++)
		sample_count += compumate_output_byte(buffer, sample_count, 0xff);

	/* data */
	for (int i = 0; i < compumate_image_size; i++)
	{
		sample_count += compumate_output_byte(buffer, sample_count, bytes[i]);
	}
/*
// save rawdata for inspection with audacity, 16bit signed data, 22050
    if (buffer)
    {
        FILE* fp=fopen("RAWDATA","wb");
        if(fp)
        {
            fwrite(buffer,sample_count*2,1,fp);
            fclose(fp);
        }
    }
*/
	return sample_count;
}


/*******************************************************************
   Generate samples for the tape image
********************************************************************/


static int compumate_bin_fill_wave(int16_t *buffer, int length, const uint8_t *bytes)
{
	return compumate_bin_handle_cassette(buffer, bytes);
}

/*******************************************************************
   Calculate the number of samples needed for this tape image
********************************************************************/

static int compumate_bin_calculate_size_in_samples(const uint8_t *bytes, int length)
{
	compumate_image_size = length;

	return compumate_bin_handle_cassette(nullptr, bytes);
}

//*********************************************************************************

static const cassette_image::LegacyWaveFiller compumate_bin_fill_intf =
{
	compumate_bin_fill_wave,                 /* fill_wave */
	-1,                                      /* chunk_size */
	0,                                       /* chunk_samples */
	compumate_bin_calculate_size_in_samples, /* chunk_sample_calc */
	COMPUMATE_WAV_FREQUENCY,                 /* sample_frequency */
	0,                                       /* header_samples */
	0                                        /* trailer_samples */
};

static cassette_image::error compumate_bin_identify(cassette_image *cassette, cassette_image::Options *opts)
{
	return cassette->legacy_identify(opts, &compumate_bin_fill_intf);
}

static cassette_image::error compumate_bin_load(cassette_image *cassette)
{
	return cassette->legacy_construct(&compumate_bin_fill_intf);
}

static const cassette_image::Format compumate_bin_format =
{
	"bin",
	compumate_bin_identify,
	compumate_bin_load,
	nullptr
};

//*********************************************************************************


//*********************************************************************************

CASSETTE_FORMATLIST_START(compumate_cassette_formats)
	CASSETTE_FORMAT(compumate_bin_format)
CASSETTE_FORMATLIST_END

