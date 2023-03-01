#include "file_access_nx.h"

#include "core/os/os.h"

#include <nn/nn_Result.h>

Error FileAccessNX::FileAccessNX::open_internal(const String &p_path, int p_mode_flags)
{
    nx_cache_start_pos = -1;
    nx_cache_end_pos = -1;
    nx_file_size = 0;

    _close();

    offset = 0;
    path_src = p_path;
    path = p_path;

    int mode = 0;

    ERR_FAIL_COND_V(f.handle, ERR_ALREADY_IN_USE);

    if (p_mode_flags == READ)
		mode |= nn::fs::OpenMode_Read;
	else if (p_mode_flags == WRITE) {
		mode |= nn::fs::OpenMode_Write;
        mode |= nn::fs::OpenMode_AllowAppend;
	} else if (p_mode_flags == READ_WRITE) {
		mode |= nn::fs::OpenMode_Read;
        mode |= nn::fs::OpenMode_Write;
        mode |= nn::fs::OpenMode_AllowAppend;
    } else if (p_mode_flags == WRITE_READ) {
		mode |= nn::fs::OpenMode_Read;
        mode |= nn::fs::OpenMode_Write;
        mode |= nn::fs::OpenMode_AllowAppend;
    } else
		return ERR_INVALID_PARAMETER;

	nn::Result result;

    // If mode is write, and to user data, flag for special close handling
    const String user_data_path = OS::get_singleton()->get_user_data_dir();
    if (p_mode_flags & WRITE) {
        if (p_path.begins_with(user_data_path)) {
            is_writable_user_data = true;
        } else {
            is_writable_user_data = false;
        }
        if (p_path.begins_with(OS::get_singleton()->get_cache_path())) {
            is_writable_cache_data = true;
        } else {
            is_writable_cache_data = false;
        }
    }

	// If mode is write, and the file does not exist, create it
	if ((p_mode_flags & WRITE) && !file_exists(path)) {
		result = nn::fs::CreateFile(path.utf8().get_data(), 0);
		if (nn::fs::ResultPathNotFound::Includes(result)) {
			last_error = ERR_FILE_BAD_PATH;
			return last_error;
		} else if (nn::fs::ResultPathAlreadyExists::Includes(result)) {
			last_error = ERR_ALREADY_EXISTS;
			// not really an error, just means something else is wrong
			// because we shouldn't ever get here
		} else if (nn::fs::ResultUsableSpaceNotEnough::Includes(result)) {
			last_error = ERR_CANT_CREATE;
			return last_error;
		}
	}
    
    result = nn::fs::OpenFile(&f, path.utf8().get_data(), mode);
    if( nn::fs::ResultPathNotFound::Includes(result) )
    {
        // The target file cannot be found.
        // This error handling is not necessary when you are only opening existing files.
        last_error = ERR_FILE_NOT_FOUND;
        return last_error;
    } else if( nn::fs::ResultTargetLocked::Includes(result) )
    {
        // The target file is already open.
        // This error handling is not necessary when there is no possibility of the file being open already.
        last_error = ERR_FILE_ALREADY_IN_USE;
        return last_error;
    }

    last_error = OK;
    flags = mode;
    return OK;
}
	
void FileAccessNX::_close()
{
    if (!f.handle)
        return;

    nn::fs::CloseFile(f);
    if (is_writable_user_data) {
        NN_LOG("Committing user save data");
        nn::fs::Commit("user");
    } else if (is_writable_cache_data) {
        NN_LOG("Committing cache data");
        nn::fs::Commit("cache");
    }

    f.handle = NULL;
}

bool FileAccessNX::is_open() const
{
    return (f.handle != NULL);
}

String FileAccessNX::get_path() const
{
    return path_src;
}
	
String FileAccessNX::get_path_absolute() const
{
    return path;
}

void FileAccessNX::seek(uint64_t p_position)
{
    offset = p_position;
}
	
void FileAccessNX::seek_end(int64_t p_position)
{
    offset = get_length() + p_position;
}
	
uint64_t FileAccessNX::get_position() const
{
    return offset;
}

uint64_t FileAccessNX::get_length() const
{
    ERR_FAIL_COND_V(!f.handle, NULL);

    if (flags & READ) {
		if (nx_file_size > 0) {
			return nx_file_size;
		}
	}

    nn::Result result;
    result = nn::fs::GetFileSize(&nx_file_size, f);
    return uint64_t(nx_file_size);
}

bool FileAccessNX::eof_reached() const
{
    return last_error == ERR_FILE_EOF;
}

uint8_t FileAccessNX::get_8() const
{
    ERR_FAIL_COND_V(!f.handle, NULL);
    int64_t return_pos;
    // Check for eof
    if (offset >= get_length()) {
        last_error = ERR_FILE_EOF;
        return '\0';
    }

    // We use a small read cache to avoid thousands of 1 byte reads.
	// Solves issue with lot check (FS Logging)
	if(offset >= nx_cache_end_pos) {
		size_t length = NX_CACHE_SIZE;
        const uint64_t realLength = get_length();
        if (length + offset >= realLength) {
            length = realLength - offset;
        }
        nn::fs::ReadFile(f, offset, nx_cache, length);

        nx_cache_start_pos = offset;
		nx_cache_end_pos = nx_cache_start_pos + length;
	}

	return_pos = offset - nx_cache_start_pos;

    offset += 1;
	return nx_cache[return_pos];
}
	
uint64_t FileAccessNX::get_buffer(uint8_t *p_dst, uint64_t p_length) const
{
    ERR_FAIL_COND_V(!f.handle, NULL);
    // check if length + offset is longer than the file
    size_t length = p_length;
    if (length + offset >= get_length()) {
        length = get_length() - offset;
        last_error = ERR_FILE_EOF;
    }

    nn::fs::ReadFile(f, offset, p_dst, length);

    offset += length;
    return length;
}
	
Error FileAccessNX::get_error() const
{
    return last_error;
}

void FileAccessNX::flush()
{
    ERR_FAIL_COND(!f.handle);
    nn::fs::FlushFile(f);
}
	
void FileAccessNX::store_8(uint8_t p_dest)
{
    ERR_FAIL_COND(!f.handle);
	nn::Result result = nn::fs::WriteFile(f, offset, &p_dest, 1, nn::fs::WriteOption::MakeValue(nn::fs::WriteOptionFlag_Flush));
	if (nn::fs::ResultUsableSpaceNotEnough::Includes(result))
		NN_LOG("Not enough usable space to write!!\n");
    offset += 1;
}
	
void FileAccessNX::store_buffer(const uint8_t *p_src, uint64_t p_length)
{
    ERR_FAIL_COND(!f.handle);
	nn::Result result = nn::fs::WriteFile(f, offset, p_src, p_length, nn::fs::WriteOption::MakeValue(nn::fs::WriteOptionFlag_Flush));
	if (nn::fs::ResultUsableSpaceNotEnough::Includes(result))
		NN_LOG("Not enough usable space to write!!\n");
    offset += p_length;
}

void FileAccessNX::close()
{
    _close();
}

bool FileAccessNX::file_exists(const String &p_path)
{
    nn::fs::DirectoryEntryType directoryEntryType;
    nn::Result result = nn::fs::GetEntryType(&directoryEntryType, p_path.utf8().get_data());
    return !nn::fs::ResultPathNotFound().Includes(result);
}

uint64_t FileAccessNX::_get_modified_time(const String &p_file)
{
    // Not supported
    return 0;
}

uint32_t FileAccessNX::_get_unix_permissions(const String &p_file)
{
    // Not supported
    return 0;
}

Error FileAccessNX::_set_unix_permissions(const String &p_file, uint32_t p_permissions) 
{
    // Not supported
    return FAILED;
}

FileAccessNX::FileAccessNX() 
    : flags(0)
	, last_error(OK)
    , is_writable_user_data(false)
    , is_writable_cache_data(false)
    , offset(0)
{
    f.handle = NULL;
}

FileAccessNX::~FileAccessNX()
{
    _close();
}