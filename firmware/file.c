#include "integer.h"
#include "regs.h"
#include "file.h"

void file_init(struct SimpleFile * file, int i)
{
	file->num = i;
	file->size = 0;
	file->type = 0;
	file->is_readonly = 1;
	file->offset = -1;
}

DWORD cur_offset;
int cur_file;
BYTE sect_buffer[512];

BYTE cache_read(DWORD offset, int file)
{
	if(((offset & ~0x1FF) != (cur_offset & ~0x1FF)) || (cur_file != file))
	{
		int i;

		set_sd_data_mode(1);
		*zpu_out3 = offset >> 9;

		set_sd_num(file);
		set_sd_read(0);
		set_sd_read(1);
		while(!get_sd_done()) {};
		set_sd_read(0);

		set_sd_data_mode(0);
		for(i=0; i<512; i++) sect_buffer[i] = *zpu_in3;

		cur_offset = offset;
		cur_file = file;
	}
	return sect_buffer[offset & 0x1FF];
}

void file_reset()
{
	cur_file = -1;
	cur_offset = -1;
}

int file_size(struct SimpleFile * file)
{
	return file->size;
}

int file_readonly(struct SimpleFile * file)
{
	return file->is_readonly;
}

int file_type(struct SimpleFile * file)
{
	return file->type;
}

enum SimpleFileStatus file_read(struct SimpleFile *file, unsigned char *buffer, int bytes, int *bytesread)
{
	if((file->offset >= 0) && (file->size > file->offset) && (bytes > 0))
	{
		if((file->offset + bytes) > file->size) bytes = file->size - file->offset;
		*bytesread = bytes;

		while(bytes--) *buffer++ = cache_read(file->offset++, file->num); 
		return SimpleFile_OK;
	}

	*bytesread = 0;
	return SimpleFile_FAIL;
}

enum SimpleFileStatus file_seek(struct SimpleFile * file, int offsetFromStart)
{
	if((file->size > 0) && (file->size >= offsetFromStart))
	{
		file->offset = offsetFromStart;
		return SimpleFile_OK;
	}
	return SimpleFile_FAIL;
}

enum SimpleFileStatus file_write(struct SimpleFile * file, unsigned char* buffer, int bytes, int * byteswritten)
{
	return SimpleFile_FAIL;
}

enum SimpleFileStatus file_write_flush()
{
	return SimpleFile_FAIL;
}
